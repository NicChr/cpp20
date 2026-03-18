#pragma once

#include <cpp20.hpp>
using namespace cpp20;

template <typename T>
requires ((RVector<T> && RSortableType<typename T::data_type>))
[[cpp20::register]]
r_vec<r_int> test_order(T x){
  return order(x);
}


template <typename T>
requires ((RVector<T> && RSortableType<typename T::data_type>))
[[cpp20::register]]
T test_sort(T x){
  auto o = test_order(x);
  o += 1;
  return x.subset(o);
}

