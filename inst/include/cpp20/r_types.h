#ifndef CPP20_R_TYPES_H
#define CPP20_R_TYPES_H

#include <cpp20/r_concepts.h>
#include <cpp20/r_sexp.h>
#include <cpp20/r_lgl.h>
#include <cpp20/r_int.h>
#include <cpp20/r_int64.h>
#include <cpp20/r_dbl.h>
#include <cpp20/r_str.h>
#include <cpp20/r_cplx.h>
#include <cpp20/r_raw.h>
#include <cpp20/r_date.h>
#include <cpp20/r_psxct.h>
#include <cpp20/r_sym.h>

// R-based C++ types that closely align with their R equivalents
// Further methods (e.g. operators) are defined in r_scalar_methods.h
// Please note that constructing R types via e.g. r_dbl() r_int() does not account for NAs
// For any and all conversions, use the `as<>` template defined in r_coerce.h
// For example - to construct the integer 0, simply write r_int(0) or as<r_int>(0), 
// the latter being able to handle NA conversions between different types
// `as<>` is the de-facto tool for conversions between all types in cpp20

namespace cpp20 {

// Important (recursive) helper to extract the underlying NON-RVal value
// Recursively unwrap until we hit a primitive type
template <typename T>
inline constexpr auto unwrap(const T& x){
if constexpr (RVal<T>){
    return unwrap(x.value);
  } else if constexpr (RObject<T>){
    return static_cast<SEXP>(x);
  } else {
    return x;
  }
}

// // Lazy loaded version
// // R C NULL constant
// inline const r_sexp& r_null() { static const r_sexp s; return s; }
// // Blank string ''
// inline const r_str_view& r_blank_string() { static const r_str_view s; return s; };
  
// Coerce to an R type based on the C type (useful for RVal templates)
template <typename T>
inline constexpr auto as_r_val(T const& x) { 
  if constexpr (RVal<T>){
    return x;
  } else if constexpr (CastableToRVal<T>){
    return static_cast<as_r_val_t<T>>(x);
  } else {
    static_assert(
      always_false<T>,
      "Unsupported type for `as_r_val`"
    );
    return r_null;
  } 
}

template <typename T>
inline constexpr auto as_r_scalar(T const& x) { 
  if constexpr (RScalar<T>){
    return x;
  } else if constexpr (CastableToRScalar<T>){
    return as_r_scalar_t<T>(x);
  } else {
    static_assert(
      always_false<T>,
      "Unsupported type for `as_r_scalar`"
    );
    return r_null;
  } 
}

}

#endif
