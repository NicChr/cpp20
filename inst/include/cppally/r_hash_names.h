#ifndef CPPALLY_R_HASH_NAMES_H
#define CPPALLY_R_HASH_NAMES_H

#include <cppally/r_vec.h>
#include <ankerl/unordered_dense.h>
#include <optional>

namespace cppally {

namespace internal {

using r_names_map_t = ankerl::unordered_dense::map<SEXP, int>;

// Wraps a vector of unique names with a lazily-built hash map for O(1) lookup
// Assumes no duplicate names

struct hashed_names {

    r_vec<r_str_view> names;
    mutable std::optional<r_names_map_t> map;

    hashed_names() = default;
    explicit hashed_names(r_vec<r_str_view> names_) : names(std::move(names_)) {}

    void reset_map() noexcept {
        map.reset(); 
    }

    private:

    void lazy_build() const {
        if (map.has_value()) return;
        r_size_t n = names.length();

        if (n > std::numeric_limits<int>::max()) [[unlikely]] {
            abort("Long vector name hashing is not supported");
        }

        map.emplace();
        map->reserve(static_cast<std::size_t>(n));
        int n_ = n;
        for (int i = 0; i < n_; ++i){
            map->emplace(names.data()[i], i);
        }
    }

    public:

    template <RStringType T>
    r_int find(const T& name, int offset = 0) const {
        lazy_build();
        auto it = map->find(unwrap(name));
        if (it == map->end()) {
            return na<r_int>();
        }
        return r_int(it->second + offset);
    }

    // template <RStringType U>
    // r_vec<r_int> find_codes(const r_vec<U>& names) const {
    //     if (names.length() > std::numeric_limits<int>::max()) [[unlikely]] {
    //         abort("Long vector of names supplied, please use short vectors of names");
    //     }
    //     lazy_build();
    //     int n = names.length();
    //     r_vec<r_int> out(n);
    
    //     for (int i = 0; i < n; ++i){
    //       auto it = levels_hash_table->find(vals.data()[i]);
    //       out.set(i, it == levels_hash_table->end() ? na<r_int>() : r_int(it->second));
    //     }
    //     return out;
    //   }
};

}

}

#endif
