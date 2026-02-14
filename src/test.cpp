#include <cpp20.hpp>

using namespace cpp20;

void dummy1() {}
void dummy() {}

[[cpp20::register]]
r_int foo(r_int x) {
  return x + 1;
}
[[cpp20::register]]
r_vec<r_int> bar(const r_vec<r_int>& x) { 
  return x + r_int(2);
}

