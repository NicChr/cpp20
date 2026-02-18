#ifndef CPP20_R_COERCE_H
#define CPP20_R_COERCE_H

#include <cpp20/internal/r_vec.h>
#include <cpp20/internal/r_dates.h>
#include <cpp20/internal/r_posixcts.h>
#include <cpp20/internal/r_factor.h>

namespace cpp20 {

// Powerful and flexible coercion function that can handle many types and convert to R-specific C++ types and R vectors
template<typename T, typename U>
inline T as(U x) {
  if constexpr (is<U, T>){
    return x;
  } else if constexpr (RVector<U> && is<T, r_sexp>){
    return x.sexp;
  } else if constexpr (RVector<T> && is_sexp<U>){
    return internal::visit_vector(x, [&](auto xvec) -> T {
      // This will trigger the branch that checks that both are RVector
      return as<T>(xvec);
    });
  } else if constexpr (is_sexp<T> && is_sexp<U>){
    if constexpr (is<T, r_sexp>){
      // SEXP -> r_sexp
      return r_sexp(x);
    } else {
      // r_sexp -> SEXP
      return static_cast<SEXP>(x);
    }
  } else if constexpr (RVal<T> && is_sexp<U>){

      // since r_sym is a special case in general (there is no vector of symbols except for a list)
      // We check the case that r_sym is being constructed from a valid SEXP (SYMSXP)
      // Will likely remove r_sym in the future and encourage usage of r_str/r_str_view
      
      if constexpr (is<T, r_sym>){
        if (TYPEOF(x) == SYMSXP){
        return r_sym(x);
      }
    }
    
    return internal::visit_vector(x, [&](auto xvec) -> T {
      // Use branch below current branch
      return as<T>(xvec);
    });
  } else if constexpr (RVector<U> && RVal<T>){
    if (x.length() != 1){
      abort("Vector must be length-1 to be coerced to requested scalar type");
    }
    return internal::as_r<T>(x.get(0));
  } else if constexpr (RVector<T> && RVal<U>){
    using data_t = typename T::data_type;
    return r_vec<data_t>(1, internal::as_r<data_t>(x));
  } else if constexpr (RVector<U> && RVector<T>){

    using to_data_t = typename T::data_type;
    using from_data_t = typename U::data_type;

    // Special case: If both are (r_str/r_view) no need to do element conversions
    if constexpr (RStringType<to_data_t> && RStringType<from_data_t>){
      return T(unwrap(x));
    } 

    r_size_t n = x.length();
    auto out = T(n);

    if constexpr (internal::RPtrWritableType<to_data_t>){
      OMP_SIMD
      for (r_size_t i = 0; i < n; ++i){
      out.set(i, internal::as_r<to_data_t>(x.view(i)));
      }
    } else {
      for (r_size_t i = 0; i < n; ++i){
      out.set(i, internal::as_r<to_data_t>(x.view(i)));
      }
    }
    return out;
  } else if constexpr (is<T, r_factors>){
    auto str_vec = as<r_vec<r_str_view>>(x);
    auto out = r_factors(str_vec);
    return out; 
  } else if constexpr (RVal<T> && !RVector<U>) {
    return internal::as_r<T>(x);
    // If input is not an R type or an R vector type
  } else if constexpr (!RVal<U> && !RVector<U>){
    return as<T>(as_r_val(x));
  } else if constexpr (CastableToRVal<T>){
    return static_cast<T>(as<as_r_val_t<T>>(x));
  } else {
    static_assert(always_false<T>, "Unsupported type for `as`");
  }
}

// Convert any C obj to an r_vec<>
template<typename T>
inline auto as_vector(T x){
  if constexpr (RVector<T>){
    return x;
  } else if constexpr (is_sexp<T>){
    static_assert(always_false<T>, "Can't convert `SEXP/r_sexp` to `r_vec<>`, please use `as<>` to convert");
    return T();
  } else {
    auto rt_val = as_r_val(x);
    return r_vec<decltype(rt_val)>(1, rt_val);
  }
}

}

#endif
