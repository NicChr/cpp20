#include "test.h"

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

[[cpp20::register]]
r_vec<r_int> test_coerce1(const r_vec<r_sexp>& x){
  return as<r_vec<r_int>>(x);
}

[[cpp20::register]]
r_vec<r_int> test_construction(SEXP x){
  return r_vec<r_int>(x);
}

[[cpp20::register]]
r_vec<r_sexp> test_constructions(SEXP x){
  r_vec<r_sexp> out(100000);

  for (int i = 0; i < 100000; ++i){
    out.set(i, r_vec<r_int>(x));
  }
  return out;
}

[[cpp20::register]]
r_vec<r_sexp> test_constructions2(SEXP x){
  r_vec<r_sexp> out(100000);

  auto val = r_vec<r_int>(x);

  for (int i = 0; i < 100000; ++i){
    out.set(i, val);
  }
  return out;
}

[[cpp20::register]]
r_vec<r_sexp> test_constructions3(SEXP x){
  r_vec<r_sexp> out(100000);

  auto val = as<r_sexp>(r_vec<r_int>(x));

  for (int i = 0; i < 100000; ++i){
    out.set(i, val);
  }
  return out;
}

[[cpp20::register]]
r_vec<r_sexp> test_constructions4(SEXP x){
  r_vec<r_sexp> out(100000);

  auto val = r_vec<r_int>(x);

  for (int i = 0; i < 100000; ++i){
    SET_VECTOR_ELT(out, i, val);
  }
  return out;
}

[[cpp20::register]]
r_vec<r_str_view> test_set_strs(r_vec<r_str_view> x){

  r_str a = r_str(x.get(0).c_str());

  r_size_t n = x.length();

  for (r_size_t i = 0; i < n; ++i){
    x.set(i, a);
  }
  return x;
}

[[cpp20::register]]
r_vec<r_str_view> test_set_strs2(r_vec<r_str_view> x){

  SEXP a = x.view(0);

  r_size_t n = x.length();

  for (r_size_t i = 0; i < n; ++i){
    SET_STRING_ELT(x, i, a);
  }
  return x;
}
