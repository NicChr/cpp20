#ifndef CHEAPR_R_VEC_HASH_H
#define CHEAPR_R_VEC_HASH_H

#include <cpp20/internal/r_vec.h>
#define XXH_INLINE_ALL
#include <xxHash/xxhash.h>

namespace cpp20 {

namespace internal {

// Hash combine helper
inline void hash_combine(size_t& seed, size_t value) noexcept {
    seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

inline double normalise_double(double x) noexcept {
  return x + 0.0;
}

template<RVal T>
struct r_vec_hash {
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
struct r_vec_hash<r_dbl> {
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
struct r_vec_hash<r_cplx> {
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
struct r_vec_hash<r_str> {
  using is_avalanching = void; 

  [[nodiscard]] size_t operator()(const r_vec<r_str>& x) const noexcept {
    size_t seed = 0;
    r_size_t n = x.length();

    const auto *p_x = x.data();
    
    for (r_size_t i = 0; i < n; ++i) {
        SEXP str_elem = p_x[i];
        
        // Handle NA explicitly to distinguish from "NA" string
        if (str_elem == NA_STRING) {
             // Random constant to mark NA
            hash_combine(seed, 0x2e06c11c82e66601ULL);
            continue;
        }

        const char* cstr = CHAR(str_elem);
        size_t len = Rf_xlength(str_elem); 
        
        // Hash content and combine
        hash_combine(seed, XXH3_64bits(cstr, len));
    }
    return seed;
  }
};


// Specialization for lists
template<>
struct r_vec_hash<r_sexp> {
  using is_avalanching = void;

  // ---------------------------------------------------------
  // Hasher for the Map (Key = SEXP)
  // ---------------------------------------------------------

  [[nodiscard]] size_t operator()(SEXP elem) const {
      r_sexp elem_ = r_sexp(elem, internal::view_tag{});

      if (elem_.is_null()){
        // return 0x3141592653589793ULL;
        return 0;
      }

      int type = TYPEOF(elem);
      
      // Recursively hash the element
      size_t h = internal::visit_maybe_vector(elem_, [](auto vec) -> size_t {
        if constexpr (is<decltype(vec), std::nullptr_t>){
          abort("List contains non-vector element, current implementation can only hash vectors");
        } else {
          using data_t = typename decltype(vec)::data_type;
          // Recursively call r_vec_hash<T> for the inner vector
          return r_vec_hash<data_t>{}(vec);
        }
      });

      // Combine with type tag (e.g. to distinguish list(1) from list("1"))
      hash_combine(h, static_cast<size_t>(type));
      return h;
  }
  
  // ---------------------------------------------------------
  // Hasher for the Wrapper (Key = r_vec<r_sexp>)
  // ---------------------------------------------------------
  // This iterates over the list and calls the SEXP hasher above
  [[nodiscard]] size_t operator()(const r_vec<r_sexp>& lst) const noexcept {
      size_t seed = 0;
      r_size_t n = lst.length();

      for (r_size_t i = 0; i < n; ++i) {
          // Call the SEXP overload we just defined above
          size_t elem_hash = (*this)(lst.get(i));
          hash_combine(seed, elem_hash);
      }
      return seed;
  }
};


struct sexp_equal {
  bool operator()(SEXP x, SEXP y) const noexcept {
    if (x == y) return true; // same pointer
    // 16 is the flag for default exactness (ignoring bytecode differences etc)
    return static_cast<bool>(R_compute_identical(x, y, 16));
  }
};

}

}

#endif
