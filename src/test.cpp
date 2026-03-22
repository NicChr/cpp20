#include <cpp20.hpp>
using namespace cpp20;


[[cpp20::register]]
r_str cpp20_typeof(SEXP x){
  return r_str(internal::r_type_to_str(internal::CPP20_TYPEOF(x)));
}


[[cpp20::register]]
r_vec<r_sym> test_vec_of_syms(SEXP x){
  return r_vec<r_sym>(x);
}
