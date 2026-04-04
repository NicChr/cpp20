#ifndef CPP20_R_INT64_H
#define CPP20_R_INT64_H

#include <cpp20/r_concepts.h>
#include <cstdint> // For int64_t

namespace cpp20 {

// R integer64 (closely mimicking how bit64 defines it)
struct r_int64 {
    int64_t value;
    using value_type = int64_t;
    r_int64() : value{0} {}
    template <CppMathType T>
    requires (internal::can_definitely_be_int64<T>())
    explicit constexpr r_int64(T x) : value{static_cast<int64_t>(x)} {}
    constexpr operator int64_t() const { return value; }
};

}

#endif
