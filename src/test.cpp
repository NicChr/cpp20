#include <cpp20.hpp>

using namespace cpp20;

void dummy1() {}
void dummy() {}

[[cpp20::register]]
r_int foo(r_int x) {
  return x + 1;
}
int bar(int x, double y, SEXP z) {
  return x + 1;
}
