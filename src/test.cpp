#include <cpp20.hpp>
#include "test.h"

using namespace cpp20;

[[cpp20::register]]
void cpp_set_threads(int n){
  set_threads(n);
}

[[cpp20::register]]
int bar(int x){
  return foo(r_int(x)) + 1;
}

r_int cpp_get_threads(){
  return r_int(get_threads());
}

void debug_test(SEXP n) {
  using T = cpp20::r_int;
  // Does this compile?
  foo(cpp20::as<T>(n));
  // Does this compile?
  cpp20::as<cpp20::r_sexp>(1);
}
