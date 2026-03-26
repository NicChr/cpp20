#pragma once

#ifdef R_INTERNALS_H_
#if !(defined(R_NO_REMAP) && defined(STRICT_R_HEADERS))
#error R headers were included before cpp11 headers \
  and at least one of R_NO_REMAP or STRICT_R_HEADERS \
  was not defined.
#endif
#endif

#ifndef R_NO_REMAP
#define R_NO_REMAP
#endif

#ifndef STRICT_R_HEADERS
#define STRICT_R_HEADERS
#endif

#include <R_ext/Boolean.h>
#include <Rinternals.h>
#include <Rversion.h>

// clang-format off
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wattributes"
#endif

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wattributes"
#endif
// clang-format on

#include <type_traits>

#if defined(R_VERSION) && R_VERSION >= R_Version(4, 4, 0)
// Use R's new macro
#define CPP11_PRIdXLEN_T R_PRIdXLEN_T
#else
// Recreate what new R does
#ifdef LONG_VECTOR_SUPPORT
#define CPP11_PRIdXLEN_T "td"
#else
#define CPP11_PRIdXLEN_T "d"
#endif
#endif

namespace cpp11 {
namespace literals {

constexpr R_xlen_t operator""_xl(unsigned long long int value) { return value; }

}  // namespace literals

}  // namespace cpp11
