#ifndef CPP20_DECLARATIONS_HPP
#define CPP20_DECLARATIONS_HPP

#include <cpp20.hpp>
#include <exception>           // for std::exception

using namespace cpp20;

// Buffer size for error messages (matches cpp11 default)
#define CPP20_ERROR_BUFSIZE 8192

#define BEGIN_CPP20                   \
  SEXP err = R_NilValue;              \
  char buf[CPP20_ERROR_BUFSIZE] = ""; \
  try {

#define END_CPP20                                               \
  }                                                             \
  catch (cpp11::unwind_exception & e) {                         \
    err = e.token;                                              \
  }                                                             \
  catch (std::exception & e) {                                  \
    strncpy(buf, e.what(), sizeof(buf) - 1);                    \
    buf[sizeof(buf) - 1] = '\0';                                \
  }                                                             \
  catch (...) {                                                 \
    strncpy(buf, "C++ error (unknown cause)", sizeof(buf) - 1); \
    buf[sizeof(buf) - 1] = '\0';                                \
  }                                                             \
  if (buf[0] != '\0') {                                         \
    Rf_errorcall(R_NilValue, "%s", buf);                        \
  } else if (err != R_NilValue) {                               \
    R_ContinueUnwind(err);                                      \
  }                                                             \
  return R_NilValue;

#endif // CPP20_DECLARATIONS_HPP
