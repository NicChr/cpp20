#include <cpp20.hpp>
#include "test.h"

using namespace cpp20;

// Basic SEXP to SEXP
[[cpp20::register]]
SEXP cpp_test(SEXP x){
  return x;
}

[[cpp20::register]]
void cpp_set_threads(int n){
  set_threads(n);
}

[[cpp20::register]]
r_int cpp_get_threads(){
  return r_int(get_threads());
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

[[cpp20::register]]
r_vec<r_dbl> test_mix(r_int a, int b, double c, r_dbl d){
  return make_vec<r_dbl>(a, b, c, d);
}


[[cpp20::register]]
r_vec<r_str> test_mix1(r_int a, int b, double c, r_dbl d, r_str e, r_str_view f){
  return make_vec<r_str>(a, b, c, d, e, f);
}

[[cpp20::register]]
r_sym test_sym(r_sym x){
  return x;
}

[[cpp20::register]]
r_sexp test_sexp2(r_sexp x){
  return x;
}

[[cpp20::register]]
r_vec<r_sexp> test_sexp3(r_vec<r_sexp> x){
  return x;
}

[[cpp20::register]]
SEXP test_list_to_scalars(r_vec<r_sexp> x){
  return make_vec<r_sexp>(as<r_lgl>(x), as<r_int>(x), as<r_dbl>(x), make_vec<r_str>(as<r_str>(x)), as<r_sexp>(x), as<r_sym>(x));
}
