#ifndef CPP20_R_MATCH_H
#define CPP20_R_MATCH_H

#include <cpp20/internal/r_vec.h>
#include <cpp20/internal/r_vec_hash.h>

namespace cpp20 {

template <RScalar T>
r_vec<r_int> match(r_vec<T> needles, r_vec<T> haystack) {

  r_size_t n_needles = needles.length();
  r_size_t n_haystack = haystack.length();

  if (n_haystack > r_limits<r_int>::max()){
    abort("Cannot match to a long vector, please use a short vector");
  }

  // Build hash table
  ankerl::unordered_dense::map<unwrap_t<T>, int> lookup;
  lookup.reserve(n_haystack);

  for (r_size_t i = 0; i < n_haystack; ++i) {
    auto elem = unwrap(haystack.get(i));
    lookup.try_emplace(elem, static_cast<int>(i) + 1);
  }

  // Match needles
  auto out = r_vec<r_int>(n_needles);
  for (r_size_t i = 0; i < n_needles; ++i) {
    auto needle = unwrap(needles.get(i));
    auto it = lookup.find(needle);
    out.set(i, it != lookup.end() ? r_int(it->second) : na_value<r_int>());
  }

  return out;
}

// Overload for R Lists (r_vec<r_sexp>)
inline r_vec<r_int> match(r_vec<r_sexp> needles, r_vec<r_sexp> haystack) {

  r_size_t n_needles = needles.length();
  r_size_t n_haystack = haystack.length();

  if (n_haystack > r_limits<r_int>::max()){
    abort("Cannot match to a long vector, please use a short vector");
  }

  // Define Map with Custom Hash and Equality
  using MapType = ankerl::unordered_dense::map<
      SEXP,              // Key type
      int,               // Value type
      r_vec_hash<r_sexp>,
      sexp_equal
  >;

  MapType lookup;
  lookup.reserve(n_haystack);

  for (r_size_t i = 0; i < n_haystack; ++i) {
    SEXP elem = haystack.get(i);
    // try_emplace only inserts if key doesn't exist. 
    // This correctly preserves the index of the *first* match
    lookup.try_emplace(elem, static_cast<int>(i) + 1);
  }

  // Match Needles
  auto out = r_vec<r_int>(n_needles);
  
  for (r_size_t i = 0; i < n_needles; ++i) {
    SEXP needle = needles.get(i);
    auto it = lookup.find(needle);
    
    if (it != lookup.end()) {
        out.set(i, it->second);
    } else {
        out.set(i, na_value<r_int>());
    }
  }

  return out;
}

}

#endif
