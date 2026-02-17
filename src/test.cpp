#include <cpp20.hpp>
#include "test.h"

using namespace cpp20;

[[cpp20::register]]
void cpp_set_threads(int n){
  set_threads(n);
}

[[cpp20::register]]
double test1(int n){
  return n + 100;
}

[[cpp20::register]]
r_str test2(const char *x){
  return r_str(x);
}

[[cpp20::register]]
const char* test3(const char *x){
  return x;
}

[[cpp20::register]]
r_vec<r_str> test4(const char *x){
  return as<r_vec<r_str>>(x);
}


// // Auto return?
// [[cpp20::register]]
// auto test5(r_int n){
//   return n + 100;
// }

r_int cpp_get_threads(){
  return r_int(get_threads());
}
