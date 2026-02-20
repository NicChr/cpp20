#pragma once

#include <cpp20.hpp>

using namespace cpp20;

// Super permissive identity fn
template <typename T>
[[cpp20::register]]
T test_identity(T x){
  return x;
}

template <RVal T>
[[cpp20::register]]
T test_rval_identity(T x){
  return x;
}

// 1 scalar arg
template <RMathType T>
[[cpp20::register]]
r_dbl scalar1(T x){
  return as<r_dbl>(x);
}

// 1 scalar arg + more complex return value
template <RMathType T>
[[cpp20::register]]
unwrap_t<T> scalar2(T x){
  return unwrap(x);
}

// 1 vector arg
template <RMathType T>
[[cpp20::register]]
r_dbl vector1(r_vec<T> x){
  return as<r_dbl>(x.get(0));
}

// RVector template typename
template <RVector T>
[[cpp20::register]]
r_dbl vector2(T x){
  return as<r_dbl>(x.get(1));
}

// 2 scalar args (same typename)
template <RMathType T>
[[cpp20::register]]
r_dbl scalar3(T x, T y){
  return as<r_dbl>(x + y);
}
// 2 scalar args (different typenames)
template <RMathType T, RMathType U>
[[cpp20::register]]
r_dbl scalar4(T x, U y){
  return as<r_dbl>(x + y);
}
// SEXP return
template <RVector T>
[[cpp20::register]]
SEXP test_sexp(T x){
  return x.sexp;
}

template <typename T>
requires (is_sexp<T>)
[[cpp20::register]]
T test_sexp4(T x){
  return x;
}

template <typename T>
requires (is_sexp<T>)
[[cpp20::register]]
SEXP test_sexp5(r_vec<T> x){
  return x;
}

// scalar and vector args (same typenames)
template <RMathType T>
[[cpp20::register]]
r_vec<T> scalar_vec1(r_vec<T> a, T b){
  return as<r_vec<T>>(a + b);
}

// scalar and vector args (different typenames)
template <RMathType T, RMathType U>
[[cpp20::register]]
r_vec<T> scalar_vec2(r_vec<T> a, U b){
  return as<r_vec<T>>(a + b);
}
// scalar and vector args 
template <RMathType T, RMathType U>
[[cpp20::register]]
r_vec<T> scalar_vec3(r_vec<T> z, T x, U y, r_vec<U> a){
  return as<r_vec<T>>(x + y + z + a);
}
// Complicated mix of scalar, vector and plain C types
// template <RMathType T, RMathType U, typename V>
// requires (RMathType<V>)
// [[cpp20::register]]
// r_vec<T> test_mix(r_vec<T> a, double b, T c, int d, T e, U f, U g, T h, r_vec<U> i, r_vec<V> j){
//   return as<r_vec<T>>(a + b + c + d + e + f + g + h + i + j);
// }
template <RMathType T, RVector V>
requires (RMathType<typename V::data_type>)
[[cpp20::register]]
r_vec<T> test_mix2(r_vec<T> a, double b, T c, int d, T e, T f, V g){
  return as<r_vec<T>>(a + b + c + d + e + f + g);
}

// R strings
[[cpp20::register]]
inline r_vec<r_str> test_str1(r_str x){
  return as<r_vec<r_str>>(x);
}

[[cpp20::register]]
inline r_vec<r_str_view> test_str2(r_str_view x){
  return as<r_vec<r_str_view>>(x);
}
template <RStringType T>
[[cpp20::register]]
inline r_vec<r_str> test_str3(T x){
  return as<r_vec<r_str>>(x);
}
template <RStringType T>
[[cpp20::register]]
inline r_vec<r_str> test_str4(T x){
  return as<r_vec<r_str>>(x);
}

template <typename T>
[[cpp20::register]]
inline r_sym test_as_sym(T x){
  return as<r_sym>(x);
}

// R list elements

// template <typename T>
// requires (is<T, r_sexp>)
// [[cpp20::register]]
// inline r_sexp test_sexp1(T x){
//   return x;
// }
// template <typename T>
// requires (is<T, r_sexp>)
// [[cpp20::register]]
// inline r_sexp test_sexp2(T x){
//   return x;
// }

// Testing template specialisations
template <RMathType T>
[[cpp20::register]]
r_vec<T> test_specialisation(r_vec<T> x) {
  return r_vec<T>(1, T(0)); 
}

template <> 
inline r_vec<r_int> test_specialisation<r_int>(r_vec<r_int> x) { 
  return r_vec<r_int>(1, r_int(1)); 
}


template <RVal T, RVal U>
[[cpp20::register]]
auto test_coerce(r_vec<T> x, r_vec<U> ptype) {
  return as<r_vec<U>>(x);
} 
