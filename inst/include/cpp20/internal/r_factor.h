#ifndef CPP20_R_FACTOR_H
#define CPP20_R_FACTOR_H

#include <cpp20/internal/r_vec.h>
#include <cpp20/internal/r_match.h>
#include <cpp20/internal/r_unique.h>
#include <cpp20/internal/r_attrs.h>
#include <vector>

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
    if (!is_null() && !attr::inherits1(*this, "factor")){
      abort("`SEXP` must be a factor");
    }
  }

  template <RVal T>
  explicit r_factors(const r_vec<T>& x, const r_vec<T>& levels){
    auto fct = match(x, levels);
    r_vec<r_int>::operator=(std::move(fct));
    r_vec<r_str_view> str_levels;
    if constexpr (RStringType<T>){
      str_levels = r_vec<r_str_view>(unwrap(levels));
    } else {
      r_size_t n = levels.length();
      str_levels = r_vec<r_str_view>(n);
      for (r_size_t i = 0; i < n; ++i){
        str_levels.set(i, internal::as_r<r_str_view>(levels.view(i)));
      }
    }
    init_factor_attrs(str_levels);
  }

  template <RVal T>
  explicit r_factors(const r_vec<T>& x) : r_factors(x, unique(x)) {}

};

}

#endif
