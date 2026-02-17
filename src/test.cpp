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

r_int cpp_get_threads(){
  return r_int(get_threads());
}
