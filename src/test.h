#include <cpp20.hpp>

using namespace cpp20;

// template <RIntegerType T>
// requires (is<T, r_int>)
// int foo(T n){
//   return n + 1;
// }

template <typename T>
[[cpp20::register]]
int foo(T n) { return 0; }
