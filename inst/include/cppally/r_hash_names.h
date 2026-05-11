#ifndef CPPALLY_R_HASH_NAMES_H
#define CPPALLY_R_HASH_NAMES_H

#include <ankerl/unordered_dense.h>
#include <optional>
#include <memory>
#include <bit>
#include <cstdint>

namespace cppally {

namespace internal {

// Compact open-addressing table mapping SEXP keys to indices into an external
// names array. Each slot stores `index + 1` (so 0 = empty sentinel, free from
// value-init). Key comparison goes through the names array, so the table only
// needs a single int[] of `~1.5x n` ints — typically half ankerl's footprint
// and one fewer cache line touched per insert.
//
// `names_ptr_` is unowned. The owner (names_map) keeps the corresponding SEXP
// alive via r_sexp; if the owner swaps the SEXP it must call set_names_ptr.
struct sexp_index_table {

    static constexpr std::size_t MIN_CAPACITY = 16;

    sexp_index_table() = default;

    sexp_index_table(const sexp_index_table&) = delete;
    sexp_index_table& operator=(const sexp_index_table&) = delete;
    sexp_index_table(sexp_index_table&&) noexcept = default;
    sexp_index_table& operator=(sexp_index_table&&) noexcept = default;

    // Reserve room for at least n entries with ~33% headroom so a single
    // post-build append (e.g. r_factors::append_level) cannot fill the table.
    void reserve(std::size_t n, const SEXP* names_ptr) {
        std::size_t target = n + (n >> 1) + 1;
        std::size_t cap = std::bit_ceil(std::max<std::size_t>(target, MIN_CAPACITY));
        slots_ = std::make_unique<int[]>(cap); // value-init: all 0 = EMPTY
        capacity_ = cap;
        mask_ = cap - 1;
        shift_ = 64 - std::countr_zero(cap);
        size_ = 0;
        names_ptr_ = names_ptr;
    }

    // Caller must guarantee that every index already stored is valid for the new array.
    void set_names_ptr(const SEXP* names_ptr) noexcept { names_ptr_ = names_ptr; }

    // Insert `idx` into the table. The key is read from names_ptr_[idx].
    // First-wins. Returns false only if the table is completely full.
    bool insert(int idx) noexcept {
        if (size_ >= capacity_) return false;
        SEXP key = names_ptr_[idx];
        std::size_t pos = hash_(key);
        while (slots_[pos] != 0) {
            if (names_ptr_[slots_[pos] - 1] == key) return true;
            pos = (pos + 1) & mask_;
        }
        slots_[pos] = idx + 1;
        ++size_;
        return true;
    }

    int find(SEXP key) const noexcept {
        if (capacity_ == 0) return -1;
        std::size_t pos = hash_(key);
        while (slots_[pos] != 0) {
            int idx = slots_[pos] - 1;
            if (names_ptr_[idx] == key) return idx;
            pos = (pos + 1) & mask_;
        }
        return -1;
    }

    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    
    private:
    
    std::unique_ptr<int[]> slots_;
    const SEXP* names_ptr_ = nullptr;
    std::size_t capacity_ = 0;
    std::size_t mask_ = 0;
    int shift_ = 64;
    std::size_t size_ = 0;

    // Knuth multiplicative — upper bits of the product distribute well even
    // when the input bits are correlated (e.g. aligned pointers).
    std::size_t hash_(SEXP p) const noexcept {
        return (reinterpret_cast<std::uintptr_t>(p) * 0x9E3779B97F4A7C15ull) >> shift_;
    }
};

// Lazily-built hash over a STRSXP for O(1) name → index lookup.
// `names` is the protected STRSXP; nullopt = not yet captured from the parent.
// `map` is the hash table; null = not yet built from `names`.
// `map` is a shared_ptr so sibling r_vec wrappers around the same SEXP
// can share a single built table without copying — see get_or_create_cache.
struct names_map {

    std::optional<r_sexp> names;
    mutable std::shared_ptr<sexp_index_table> map;

    names_map() = default;

    void invalidate() noexcept {
        names.reset();
        map.reset();
    }

    void reset_map() noexcept {
        map.reset();
    }

    private:

    void lazy_build() const {
        if (map) return;

        map = std::make_shared<sexp_index_table>();

        if (static_cast<SEXP>(*names) == R_NilValue) return;

        r_size_t n = Rf_xlength(*names);
        if (n > std::numeric_limits<int>::max()) [[unlikely]] {
            abort("Long vector name hashing is not supported");
        }

        const SEXP* RESTRICT p_names = STRING_PTR_RO(*names);
        map->reserve(static_cast<std::size_t>(n), p_names);
        int n_ = static_cast<int>(n);
        for (int i = 0; i < n_; ++i){
            map->insert(i);
        }
    }

    public:

    template <RStringType T>
    r_int find(const T& name, int offset = 0) const {
        lazy_build();
        int idx = map->find(unwrap(name));
        if (idx < 0) {
            return na<r_int>();
        }
        return r_int(idx + offset);
    }
};

// Per-attribute cache registries: keyed by parent SEXP*, so any two wrappers
// around the same SEXP converge on the same names_map. A mutation through any
// of them is visible to all.
//
// weak_ptr storage means the cache dies when the last wrapper holding it dies.
// Dead slots are reused on next lookup or swept periodically.
//
// NOT thread-safe — assumes wrapper construction outside OMP regions.

inline ankerl::unordered_dense::map<SEXP, std::weak_ptr<names_map>>& name_cache_storage() {
    static ankerl::unordered_dense::map<SEXP, std::weak_ptr<names_map>> s;
    return s;
}

inline ankerl::unordered_dense::map<SEXP, std::weak_ptr<names_map>>& levels_cache_storage() {
    static ankerl::unordered_dense::map<SEXP, std::weak_ptr<names_map>> s;
    return s;
}

constexpr std::size_t CACHE_SWEEP_INTERVAL = 1024;

inline void sweep_cache_storage(ankerl::unordered_dense::map<SEXP, std::weak_ptr<names_map>>& storage) noexcept {
    for (auto it = storage.begin(); it != storage.end(); ) {
        if (it->second.expired()) {
            it = storage.erase(it);
        } else {
            ++it;
        }
    }
}

inline std::shared_ptr<names_map> get_or_create_cache(
    ankerl::unordered_dense::map<SEXP, std::weak_ptr<names_map>>& storage,
    SEXP s
) {
    static std::size_t counter = 0;
    // Equivalent to (++counter % CACHE_SWEEP_INTERVAL) == 0 when the interval is a power of two
    if ((++counter & (CACHE_SWEEP_INTERVAL - 1)) == 0)
        sweep_cache_storage(storage);

    auto [it, inserted] = storage.try_emplace(s);
    if (!inserted) {
        if (auto sp = it->second.lock())
            return sp;
    }
    auto sp = std::make_shared<names_map>();
    it->second = sp;
    return sp;
}
// Is there a live cache currently associated with this SEXP?
inline std::shared_ptr<names_map> try_lookup_cache(
    ankerl::unordered_dense::map<SEXP, std::weak_ptr<names_map>>& storage,
    SEXP s
) noexcept {
    auto it = storage.find(s);
    if (it != storage.end()) {
        return it->second.lock();
    }
    return nullptr;
}

inline std::shared_ptr<names_map> get_or_create_name_cache(SEXP s) {
    return get_or_create_cache(name_cache_storage(), s);
}

inline std::shared_ptr<names_map> get_or_create_levels_cache(SEXP s) {
    return get_or_create_cache(levels_cache_storage(), s);
}

inline std::shared_ptr<names_map> try_lookup_name_cache(SEXP s) noexcept {
    return try_lookup_cache(name_cache_storage(), s);
}

inline std::shared_ptr<names_map> try_lookup_levels_cache(SEXP s) noexcept {
    return try_lookup_cache(levels_cache_storage(), s);
}

}

}

#endif
