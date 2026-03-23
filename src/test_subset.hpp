#pragma once

#include <cpp20.hpp>
using namespace cpp20;

template <RShortIntegerType T>
[[cpp20::register]]
r_vec<T> test_exclude_locs(r_int n, r_vec<T> x){
  return internal::exclude_locs(x, n);
}
template <RVal T, internal::RSubscript U>
[[cpp20::register]]
SEXP test_clean_locs(r_vec<U> locs, r_vec<T> x){
    return clean_locs(locs, x);
}


template <RVector T>
[[cpp20::register]]
T test_replace(T x, r_vec<r_int> y, T z){
    auto out = deep_copy(x);
    out.fill(y, z);
    return out;
}

template <RVal T>
[[cpp20::register]]
r_vec<T> test_replace2(r_vec<T> x, r_int from, r_int n, T val){
    r_vec<T> out = deep_copy(x);
    out.fill(from, n, val);
    return out;
}

template <RVector T, typename U>
requires (any<U, r_int, r_lgl, r_str, r_str_view>)
[[cpp20::register]]
T test_subset(T x, r_vec<U> i){
    return x.subset(i);
}
