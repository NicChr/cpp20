#ifndef CPP20_R_RECYCLE_H
#define CPP20_R_RECYCLE_H

#include <cpp20/r_visit.h>
#include <cpp20/r_list_helpers.h>
#include <cpp20/sugar/r_make_vec.h>

namespace cpp20 {

// Variadic recycle function
template <typename... Args>
inline r_vec<r_sexp> recycle(Args&&... args) {
  r_vec<r_sexp> out = make_vec<r_sexp>(std::forward<Args>(args)...);
  internal::recycle_impl(out, internal::recycle_size(out));
  return out;
}

}

#endif
