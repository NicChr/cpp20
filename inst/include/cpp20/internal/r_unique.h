#ifndef CPP20_R_UNIQUE_H
#define CPP20_R_UNIQUE_H

#include <cpp20/internal/r_vec_math.h>
#include <cpp20/internal/r_groups.h>

namespace cpp20 {

template <RVal T>
r_vec<T> unique(const r_vec<T>& x) {
    auto group_info = make_groups(x);
    auto starts = group_starts(group_info);
    starts += 1;
    return x.subset(starts);
}

// template <RScalar T>
// r_vec<T> unique(const r_vec<T>& x) {
//   r_size_t n = x.length();
  
//   // Hash set for O(n) de-duplication
//   ankerl::unordered_dense::set<
//     unwrap_t<T>, 
//     internal::r_hash<T>, 
//     internal::r_hash_eq<T>
//   > seen;
//   seen.reserve(n);
  
//   // Store uniques in insertion order using raw SEXP pointers
//   std::vector<unwrap_t<T>> uniques;
//   uniques.reserve(n);
  
//   auto *p_x = x.data();
  
//   for (r_size_t i = 0; i < n; ++i) {
//       auto elem = p_x[i];
//       if (seen.insert(elem).second) {
//           uniques.push_back(elem);
//       }
//   }
  
//   r_size_t n_unique = uniques.size();
  
//   if (n_unique == n) {
//       return x;
//   }
  
//   auto out = r_vec<T>(n_unique);
//   for (r_size_t i = 0; i < n_unique; ++i) {
//       out.set(i, T(uniques[i]));
//   }
  
//   return out;
// }

// // Specialisation for strings
// template <>
// r_vec<r_str> unique(const r_vec<r_str>& x) {
//   r_size_t n = x.length();
  
//   // Hash set for O(n) de-duplication
//   ankerl::unordered_dense::set<SEXP> seen;
//   seen.reserve(n);
  
//   // Store uniques in insertion order using raw SEXP pointers
//   std::vector<SEXP> uniques;
//   uniques.reserve(n);
  
//   auto *p_x = x.data();
  
//   for (r_size_t i = 0; i < n; ++i) {
//       SEXP str = p_x[i];
//       if (seen.insert(str).second) {
//           uniques.push_back(str);
//       }
//   }
  
//   r_size_t n_unique = uniques.size();
  
//   if (n_unique == n) {
//       return x;
//   }
  
//   auto out = r_vec<r_str>(n_unique);
//   for (r_size_t i = 0; i < n_unique; ++i) {
//       SET_STRING_ELT(out, i, uniques[i]);
//   }
  
//   return out;
// }

}

#endif
