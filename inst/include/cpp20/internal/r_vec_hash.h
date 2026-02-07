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

template<RVal T>
struct r_vec_hash {
  using is_avalanching = void;
  
  [[nodiscard]] size_t operator()(const r_vec<T>& x) const noexcept {
    r_size_t n = x.length();
    auto* RESTRICT data = x.data();
    return XXH3_64bits(data, n * sizeof(unwrap_t<T>));
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

  [[nodiscard]] size_t operator()(SEXP elem) const noexcept {
      r_sexp elem_ = r_sexp(elem, internal::read_only_tag{});
      int type = TYPEOF(elem);
      
      // Recursively hash the element
      size_t h = internal::visit_vector(elem_, [](auto vec) -> size_t {
          using data_t = typename decltype(vec)::data_type;
          // Recursively call r_vec_hash<T> for the inner vector
          return r_vec_hash<data_t>{}(vec);
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
    return R_compute_identical(x, y, 16); 
  }
};

}

}

#endif
