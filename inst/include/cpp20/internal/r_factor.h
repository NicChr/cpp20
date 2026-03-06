#ifndef CPP20_R_FACTOR_H
#define CPP20_R_FACTOR_H

#include <cpp20/internal/r_vec.h>
#include <cpp20/internal/r_attrs.h>

namespace cpp20 {

struct r_factors : public r_vec<r_int> { 

  public: 

  r_vec<r_str_view> levels() const {
    return r_vec<r_str_view>(attr::get_attr(*this, r_sym("levels")));
  }

  template <RStringType T>
  void set_levels(const r_vec<T>& levels) {
    attr::set_attr(*this, r_sym("levels"), levels);
  }

  private:


  explicit r_factors(r_vec<r_int>&& x) : r_vec<r_int>(std::move(x)) {}

  void validate_factor(SEXP x){
    if (TYPEOF(x) != INTSXP){
      abort("SEXP must have integer storage to be constructed as a factor");
    }
    if (!Rf_inherits(x, "factor")){
      abort("SEXP must be of class 'factor' to be constructed as a factor");
    }
  }

  template <RStringType T>
  void init_factor_attrs(const r_vec<T>& levels) {
      // Set class
      attr::set_attr(*this, r_sym("class"), r_vec<r_str_view>(1, "factor"));
      // Set levels
      set_levels(levels);
  }

  public: 

  // Constructors
  r_factors() : r_vec<r_int>() {
    init_factor_attrs(r_vec<r_str_view>());
  }

  explicit r_factors(SEXP x) : r_vec<r_int>(x) {
    if (!is_null()){
      validate_factor(x); 
    }
  }

  explicit r_factors(SEXP x, internal::view_tag) : r_vec<r_int>(x, internal::view_tag{}) {
    if (!is_null()){
      validate_factor(x); 
    }
  }

  explicit r_factors(r_size_t n): r_vec<r_int>(n, na<r_int>()){
    init_factor_attrs(r_vec<r_str_view>());
  }

  template <RVal T>
  explicit r_factors(const r_vec<T>& x, const r_vec<T>& levels);
  
  template <RVal T>
  explicit r_factors(const r_vec<T>& x);

  template <typename U>
  r_factors subset(const r_vec<U>& indices) const {
    r_factors out(r_vec<r_int>::subset(indices)); 
    out.init_factor_attrs(levels());
    return out;
}

  template <typename U>
  r_factors subset(U index) const {
    r_factors out(r_vec<r_int>::subset(index)); 
    out.init_factor_attrs(levels());
    return out;
  }

  r_factors resize(r_size_t n) {
    r_factors out(r_vec<r_int>::resize(n)); 
    out.init_factor_attrs(levels());
    return out;
  }
  r_factors rep_len(r_size_t n) {
    r_factors out(r_vec<r_int>::rep_len(n)); 
    out.init_factor_attrs(levels());
    return out;
  }

};

}

#endif
