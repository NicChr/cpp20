#pragma once

#include <cpp20.hpp>
using namespace cpp20;

template <RVector T, typename U>
requires (any<U, r_int, r_lgl, r_str, r_str_view>)
[[cpp20::register]]
T test_subset(T x, r_vec<U> i){
    return x.subset(i);
}
