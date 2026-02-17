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
