#ifndef CPP20_R_DBL_H
#define CPP20_R_DBL_H

#include <cpp20/r_concepts.h>

namespace cpp20 {

// R double
struct r_dbl {
  double value;
  using value_type = double;
  r_dbl() : value{0} {}
  template <CppMathType T>
  explicit constexpr r_dbl(T x) : value{static_cast<double>(x)} {}
  constexpr operator double() const { return value; }
};

}

#endif
