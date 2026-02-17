#pragma once

#include <cpp20.hpp>

using namespace cpp20;

template <RMathType T>
[[cpp20::register]]
r_dbl foo(T x){
  return as<r_dbl>(x);
}

template <RMathType T>
[[cpp20::register]]
unwrap_t<T> foo2(T x){
  return unwrap(x);
}

template <RMathType T>
[[cpp20::register]]
r_dbl bar(r_vec<T> x){
  return as<r_dbl>(x.get(0));
}

template <RVector T>
[[cpp20::register]]
r_dbl foobar(T x){
  return as<r_dbl>(x.get(1));
}

// template <RMathType U>
// r_vec<r_dbl> foo3(r_vec<U> x){
//   return as<r_vec<r_dbl>>(x);
// }

template <RMathType T, RMathType U>
[[cpp20::register]]
r_dbl foo3(T x, U y){
  return as<r_dbl>(x + y);
}

template <RVector T>
[[cpp20::register]]
SEXP foo4(T x){
  return x.sexp;
}

// template <RMathType T, RMathType U>
// [[cpp20::register]]
// r_vec<unwrap_t<T>> foo4(r_vec<T> z, T x, U y, r_vec<U> a){
//   return as<r_vec<T>>(x + y + z + a);
// }

// template <RMathType T, RMathType U>
// [[cpp20::register]]
// unwrap_t<T> foo4(int z, T x, U y, double a){
//   return as<T>(x + y + z + a);
// }
