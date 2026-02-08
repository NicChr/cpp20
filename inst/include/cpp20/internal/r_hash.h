#ifndef CPP20_R_HASH_H
#define CPP20_R_HASH_H

#include <cpp20/internal/r_nas.h>
#include <bit>
#define XXH_INLINE_ALL
#include <xxHash/xxhash.h>

// Defining hash functions + hash equality operators

namespace cpp20 {

namespace internal {

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

template <typename T>
struct r_hash_impl : xxh3_base {
    
    // This tells Ankerl map 'this hash is already high quality'
    using is_avalanching = void;

    uint64_t operator()(const T& x) const noexcept {
        return hash_bytes(&x, sizeof(x));
    }
};

template <>
struct r_hash_impl<double> : xxh3_base {
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
struct r_hash_impl<std::complex<double>> : xxh3_base {
    using is_avalanching = void;

    uint64_t operator()(std::complex<double> x) const noexcept {
        r_hash_impl<double> hasher;
        // Hash real and imag parts and mix
        uint64_t h1 = hasher(x.real());
        uint64_t h2 = hasher(x.imag());
        return combine(h1, h2);
    }
};

template <typename T>
struct r_hash_eq_impl {
    using is_transparent = void;
    bool operator()(const T& a, const T& b) const noexcept {
        return a == b;
    }
};

template <>
struct r_hash_eq_impl<double> {
    using is_transparent = void;
    bool operator()(double a, double b) const noexcept {
        if (std::isnan(a) && std::isnan(b)){
            return true;
        } else {
            return a == b;
        }
    }
};

template <>
struct r_hash_eq_impl<std::complex<double>> {
    using is_transparent = void;
    bool operator()(std::complex<double> a, std::complex<double> b) const noexcept {
        r_hash_eq_impl<double> eq;
        return eq(a.real(), b.real()) && eq(a.imag(), b.imag());
    }
};

template <RScalar T>
struct r_hash : r_hash_impl<unwrap_t<T>> {};
template <RScalar T>
struct r_hash_eq : r_hash_eq_impl<unwrap_t<T>> {};


}

}

#endif
