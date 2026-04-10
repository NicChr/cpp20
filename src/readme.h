#pragma once

#include <cpp20.hpp>
using namespace cpp20;

// What type is deduced by dispatch?
template <typename T>
[[cpp20::register]]
T scalar_init(T ptype){
	return T();
}

[[cpp20::register]]
r_int64 as_int64(r_int x){
	return as<r_int64>(x);
}

[[cpp20::register]]
r_size_t as_r_size_t(r_int x){
	return as<r_size_t>(x);
}

template <RVector T>
[[cpp20::register]]
int cpp_length(T vec){
	return vec.length();
}

[[cpp20::register]]
r_dbl add_half(r_dbl x){
  return x + 0.5;
}

[[cpp20::register]]
r_str symbol_to_string(r_sym x){
	return as<r_str>(x);
}

[[cpp20::register]]
r_factors new_factor(r_vec<r_str> x){
	return r_factors(x);
}

static_assert(!RVector<r_factors>);

[[cpp20::register]]
r_vec<r_int> factor_codes(r_factors x){
	return x.codes();
}

[[cpp20::register]]
r_vec<r_int> cpp_lengths(const r_vec<r_sexp>& x){
  r_size_t n = x.length();
  r_vec<r_int> out(n); // Initialise lengths vector
  
    for (r_size_t i = 0; i < n; ++i){
       visit_vector(x.view(i), [&](const auto& vec) {
         out.set(i, as<r_int>(vec.length()));
    });
      
    }

  return out;
}

[[cpp20::register]]
r_vec<r_int> cpp_lengths2(const r_vec<r_sexp>& x){
    r_size_t n = x.length();
    r_vec<r_int> out(n); // Initialise lengths vector
    
      for (r_size_t i = 0; i < n; ++i){
         visit_sexp(x.view(i), [&](const auto& vec) {
         using vec_t = decltype(vec);
         
         if constexpr (!RVector<vec_t>){
             abort("Input must be an RVector");
         } else {
             out.set(i, as<r_int>(vec.length()));
         }
      });
        
      }
    return out;
}

// Coerces NA correctly
[[cpp20::register]]
r_int double_to_int(r_dbl x){
  return as<r_int>(x);
}

[[cpp20::register]]
r_vec<r_int> to_int_vec(r_vec<r_dbl> x){
  return as<r_vec<r_int>>(x);
}

[[cpp20::register]]
r_vec<r_sexp> coercions(){
  r_dbl a(4.2);
  r_vec<r_dbl> b = make_vec<r_dbl>(2.5); // Vector containing 2.5
  
  return make_vec<r_sexp>(
    as<r_vec<r_int>>(a),
    as<r_int>(a),
    as<r_int>(b),
    as<r_dbl>(b)
  );
}
