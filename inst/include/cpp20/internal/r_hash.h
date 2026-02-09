#ifndef CPP20_R_HASH_H
#define CPP20_R_HASH_H

#include <cpp20/internal/r_nas.h>
#include <cpp20/internal/r_vec.h>
#include <cpp20/internal/r_attrs.h>
#include <bit>
#define XXH_INLINE_ALL
#include <xxHash/xxhash.h>

// Defining hash functions + hash equality operators

namespace cpp20 {

namespace internal {

// Hash combine helper
inline uint32_t hash_combine(uint64_t seed, uint64_t value) noexcept {
    return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
} 

// High quality 64-bit mixer from murmurhash
inline uint64_t mix_u64(uint64_t x) noexcept {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

struct xxh3_base {
    // Standard mixer: simply delegates to XXH3
    static inline uint64_t hash_bytes(const void* data, size_t len) {
        return XXH3_64bits(data, len);
    }
    
    // Fast path for 64-bit primitives (like double/int64)
    static inline uint64_t hash_u64(uint64_t k) {
        return XXH3_64bits(&k, sizeof(k));
    }

    static inline uint64_t combine(uint64_t h1, uint64_t h2) {
        uint64_t buffer[2] = {h1, h2};
        return XXH3_64bits(buffer, sizeof(buffer));
    }
};

template <RVal T>
struct r_hash_impl : xxh3_base {
    
    // This tells Ankerl map 'this hash is already high quality'
    using is_avalanching = void;

    uint64_t operator()(const unwrap_t<T>& x) const noexcept {
        return XXH3_64bits(&x, sizeof(x));
    }
};
template <>
struct r_hash_impl<r_int> {
    using is_avalanching = void;
    uint64_t operator()(int x) const noexcept {
        return mix_u64(static_cast<uint64_t>(x));
    }
};
template <>
struct r_hash_impl<r_int64> {
    using is_avalanching = void;
    uint64_t operator()(int64_t x) const noexcept {
        return mix_u64(static_cast<uint64_t>(x));
    }
};
template <>
struct r_hash_impl<r_dbl> : xxh3_base {
    using is_avalanching = void;

    uint64_t operator()(double x) const noexcept {
        // Checks that x matches exactly to R's NA_REAL
        if (is_na_real(x)){
            return hash_u64(na_real_bits());
        // Checks all other NaN types
        } else if (x != x){
            return hash_u64(nan_bits());
        } else {
            // Hash normal double
            // +0.0 to normalise -0.0 and 0.0 
            // Standard XXH3 on bits would hash them differently.
            return hash_u64(std::bit_cast<uint64_t>(x + 0.0));
        }
    }
};

template <>
struct r_hash_impl<r_cplx> : xxh3_base {
    using is_avalanching = void;

    uint64_t operator()(std::complex<double> x) const noexcept {
        r_hash_impl<r_dbl> hasher;
        // Hash real and imag parts and mix
        uint64_t h1 = hasher(x.real());
        uint64_t h2 = hasher(x.imag());
        return combine(h1, h2);
    }
};

template<>
struct r_hash_impl<r_str> {
    using is_avalanching = void;

    auto operator()(SEXP x) const noexcept -> uint64_t {
        // Cast pointer to integer (uintptr_t)
        auto ptr_val = reinterpret_cast<uintptr_t>(x);
        
        // Scramble the bits
        // We use ankerl's built-in wyhash mixer. It's just a multiply + XOR.
        return ankerl::unordered_dense::detail::wyhash::hash(ptr_val);
    }
};

    
inline double normalise_double(double x) noexcept {
    return x + 0.0;
}


// Vector hashing

template<RVal T>
struct r_vec_hash_impl {
    using is_avalanching = void;
    
    [[nodiscard]] size_t operator()(const r_vec<T>& x) const noexcept {
    r_size_t n = x.length();
    auto* RESTRICT data = x.data();
    return XXH3_64bits(data, n * sizeof(unwrap_t<T>));
    }
};

// Specialisations for r_dbl/r_cplx
// Normalise doubles to ensure -0.0 and 0.0 aren't hashed differently

template<>
struct r_vec_hash_impl<r_dbl> {
    using is_avalanching = void;

    [[nodiscard]] size_t operator()(const r_vec<r_dbl>& x) const noexcept {
    r_size_t n = x.length();
    const double* data = x.data();

    // Use streaming API to avoid allocating a full copy of the vector
    XXH3_state_t* state = XXH3_createState();
    XXH3_64bits_reset(state);

    constexpr size_t CHUNK_SIZE = 1024; // 8KB buffer, fits in stack/L1
    std::array<double, CHUNK_SIZE> buffer;

    for (r_size_t i = 0; i < n; i += CHUNK_SIZE) {
        size_t current_chunk = std::min<size_t>(CHUNK_SIZE, n - i);
        
        // Copy and normalise into stack buffer
        for (size_t j = 0; j < current_chunk; ++j) {
            buffer[j] = normalise_double(data[i + j]);
        }
        
        XXH3_64bits_update(state, buffer.data(), current_chunk * sizeof(double));
    }

    size_t hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return hash;
    }
};

template<>
struct r_vec_hash_impl<r_cplx> {
    using is_avalanching = void;

    [[nodiscard]] size_t operator()(const r_vec<r_cplx>& x) const noexcept {
    r_size_t n = x.length();
    const auto* data = x.data();

    XXH3_state_t* state = XXH3_createState();
    XXH3_64bits_reset(state);

    constexpr size_t CHUNK_SIZE = 1024; 
    std::array<std::complex<double>, CHUNK_SIZE> buffer;

    for (r_size_t i = 0; i < n; i += CHUNK_SIZE) {
        size_t current_chunk = std::min<size_t>(CHUNK_SIZE, n - i);
        
        for (size_t j = 0; j < current_chunk; ++j) {
            double r = normalise_double(data[i + j].real());
            double im = normalise_double(data[i + j].imag());
            buffer[j] = std::complex<double>(r, im);
        }
        
        XXH3_64bits_update(state, buffer.data(), current_chunk * sizeof(std::complex<double>));
    }

    size_t hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return hash;
    }
};

// String vector - hash each CHARSXP individually then combine
template<>
struct r_vec_hash_impl<r_str> {
  using is_avalanching = void; 

  [[nodiscard]] size_t operator()(const r_vec<r_str>& x) const noexcept {
    r_size_t n = x.length();
    const auto *p_x = x.data();
    
    // Create state once
    XXH3_state_t* state = XXH3_createState();
    XXH3_64bits_reset(state);

    for (r_size_t i = 0; i < n; ++i) {
        SEXP str_elem = p_x[i];
        
        if (str_elem == NA_STRING) {
            uint64_t na_marker = 0x2e06c11c82e66601ULL;
            // Feed marker into stream
            XXH3_64bits_update(state, &na_marker, sizeof(na_marker));
        } else {
            const char* cstr = CHAR(str_elem);
            size_t len = Rf_xlength(str_elem);
            // Feed the string bytes directly into the main stream
            // But we need a delimiter to avoid "ab", "c" hashing same as "a", "bc"
            // Update with length, then bytes
            XXH3_64bits_update(state, &len, sizeof(len));
            XXH3_64bits_update(state, cstr, len);
        }
    }
    
    size_t hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return hash;
  }
};

// Specialization for lists
template<>
struct r_hash_impl<r_sexp> {
    using is_avalanching = void;

    [[nodiscard]] size_t operator()(SEXP x) const {

        auto x_ = r_sexp(x, internal::view_tag{});

        if (x_.is_null()) return 0;

        size_t seed = 0;

        int type = TYPEOF(x);
        
        // Recursively hash the element
        size_t h = internal::visit_maybe_vector(x_, [this, seed](const auto& vec) -> size_t {

            size_t seed_ = seed;

            using vec_t = std::remove_cvref_t<decltype(vec)>;
            
            if constexpr (is<vec_t, std::nullptr_t>){
            abort("List contains non-vector element, current implementation can only hash vectors");
        } else {
            using data_t = typename vec_t::data_type;

            if constexpr (is<data_t, r_sexp>){
                r_size_t n = vec.length();
                for (r_size_t i = 0; i < n; ++i) {
                    size_t elem_hash = (*this)(vec.view(i));
                    seed_ = hash_combine(seed_, elem_hash);
                }
                return seed_;
            } else {
                return r_vec_hash_impl<data_t>{}(vec);
            }
        }
        });
        return hash_combine(h, static_cast<size_t>(type));
    }
};


template <RVal T>
struct r_hash_eq_impl {
    using is_transparent = void;
    bool operator()(const unwrap_t<T>& a, const unwrap_t<T>& b) const noexcept {
        return a == b;
    }
};

template <>
struct r_hash_eq_impl<r_dbl> {
    using is_transparent = void;
    bool operator()(double a, double b) const noexcept {
        if (std::isnan(a) && std::isnan(b)){
            return is_na_real(a) == is_na_real(b);
        } else {
            return a == b;
        }
    }
};

template <>
struct r_hash_eq_impl<r_cplx> {
    using is_transparent = void;
    bool operator()(std::complex<double> a, std::complex<double> b) const noexcept {
        r_hash_eq_impl<r_dbl> eq;
        return eq(a.real(), b.real()) && eq(a.imag(), b.imag());
    }
};

template<>
struct r_hash_eq_impl<r_str> {
    using is_transparent = void;
    bool operator()(SEXP a, SEXP b) const noexcept {
        return a == b;
    }
};

// Meant to be used for elements of lists
// Since they are r_sexp we have to 'visit' the type of vector it is
template<>
struct r_hash_eq_impl<r_sexp> {
    bool operator()(SEXP x, SEXP y) const {
    if (x == y) return true; // same pointer
    if (Rf_xlength(x) != Rf_xlength(y)) return false;
    if (TYPEOF(x) != TYPEOF(y)) return false;
    
    bool x_has_attrs = attr::has_attrs(r_sexp(x, internal::view_tag{}));
    bool y_has_attrs = attr::has_attrs(r_sexp(y, internal::view_tag{}));
    if (x_has_attrs != y_has_attrs) return false;
    
    if (x_has_attrs && y_has_attrs){
        if (!r_hash_eq_impl<r_sexp>{}(attr::get_attrs(r_sexp(x, internal::view_tag{})), attr::get_attrs(r_sexp(y, internal::view_tag{})))){
            return false;
        }
    }

    return internal::visit_maybe_vector(x, [y](auto vec) -> bool {
        using vec_t = decltype(vec);

        if constexpr (is<vec_t, std::nullptr_t>){
            abort("List contains non-vector element, current implementation can only hash vectors");
        } else {
            using data_t = typename vec_t::data_type;
            if constexpr (any<data_t, r_str, r_sym>){
                // We already checked whether pointer addresses match in the first line
                // Since they don't match, they aren't equal for strings & symbols
                return false;
            } else if constexpr (is<data_t, r_sexp>) {
                r_size_t n = vec.length();
                for (r_size_t i = 0; i < n; ++i){
                    if (!r_hash_eq_impl<r_sexp>{}(VECTOR_ELT(vec, i), VECTOR_ELT(y, i))){
                        return false;
                    }
                }
                return true;
            } else {
                return std::memcmp(DATAPTR_RO(vec), DATAPTR_RO(y), vec.size() * sizeof(unwrap_t<data_t>)) == 0; 
            }
        }
        });
    }
};

template <RVal T>
struct r_hash : r_hash_impl<std::remove_cvref_t<T>> {};
template <RVal T>
struct r_hash_eq : r_hash_eq_impl<std::remove_cvref_t<T>> {};

}

}

#endif
