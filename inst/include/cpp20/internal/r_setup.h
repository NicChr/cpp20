#ifndef CPP20_R_SETUP_H
#define CPP20_R_SETUP_H

#ifndef R_NO_REMAP
#define R_NO_REMAP 
#endif

#include <cpp11/sexp.hpp> // For cpp11::sexp
#include <type_traits> // For concepts
#include <ankerl/unordered_dense.h> // Hash maps for group IDs + unique + match
#include <cstring> // For strcmp
#include <string> // For C++ strings
#include <charconv> // For to_chars
#include <complex> // For complex<double>
#include <limits> // For numeric limits
#include <vector> // For C++ vectors
#include <algorithm> // For sort + other utilities
#include <bit> // For bit_cast
#include <ska_sort/ska_sort.hpp> // For radix sorting via ska_sort
#include <chrono> // For r_date/r_psxt

#ifdef _MSC_VER
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

#if !defined(OBJSXP) && defined(S4SXP) 
#define OBJSXP S4SXP
#endif

#ifdef _OPENMP
#include <omp.h>
#define OMP_PRAGMA(x) _Pragma(#x)
#define OMP_NUM_PROCS omp_get_num_procs()
#define OMP_THREAD_LIMIT omp_get_thread_limit()
#define OMP_MAX_THREADS omp_get_max_threads()
#define OMP_PARALLEL(n_threads) OMP_PRAGMA(omp parallel if ((n_threads) > 1) num_threads((n_threads)))
#define OMP_FOR_SIMD OMP_PRAGMA(omp for simd)
#define OMP_SIMD OMP_PRAGMA(omp simd)
#define OMP_PARALLEL_FOR_SIMD(n_threads) OMP_PRAGMA(omp parallel for simd if ((n_threads) > 1) num_threads((n_threads)))

#define OMP_DO_NOTHING
#else
#define OMP_PRAGMA(x)
#define OMP_NUM_PROCS 1
#define OMP_THREAD_LIMIT 1
#define OMP_MAX_THREADS 1
#define OMP_PARALLEL(n_threads)
#define OMP_SIMD
#define OMP_FOR_SIMD
#define OMP_PARALLEL_FOR_SIMD(n_threads)
#define OMP_DO_NOTHING
#endif

namespace cpp20 {
    
using r_size_t = R_xlen_t;

namespace internal {

// using r_sexp_tag_t = uint16_t; // cpp20 version of SEXPTYPE

// Currently unfinished
// enum : r_sexp_tag_t {
//     r_lgl_id = 1,
//     r_int_id = 2,
//     r_int64_id = 3,
//     r_dbl_id = 4,
//     r_cplx_id = 5,
//     r_raw_id = 6,
//     r_dates_id = 7,
//     r_pxt_id = 8,
//     r_chr = 9,
//     r_fct = 10,
//     r_list = 11,
//     r_df = 12,
//     r_unk = 13
// };


// Custom SEXP tags, differentiating integer64, dates (int/double), date-times (int64/double) and factors
inline constexpr int64_t CPP20_OMP_THRESHOLD = 100000;
inline constexpr SEXPTYPE CPP20_INT64SXP = 64;
inline constexpr SEXPTYPE CPP20_INTDATESXP = 200;
inline constexpr SEXPTYPE CPP20_REALDATESXP = 201;
inline constexpr SEXPTYPE CPP20_INT64PSXTSXP = 202;
inline constexpr SEXPTYPE CPP20_REALPSXTSXP = 203;
inline constexpr SEXPTYPE CPP20_FCTSXP = 204;
inline constexpr SEXPTYPE CPP20_DFSXP = 205;
inline int cpp20_n_threads = 1;

[[noexcept]] inline SEXPTYPE CPP20_TYPEOF(SEXP x){

  auto xtype = TYPEOF(x);

  switch (xtype){
    case INTSXP: {
      if (Rf_inherits(x, "Date")) return CPP20_INTDATESXP;
      if (Rf_inherits(x, "factor")) return CPP20_FCTSXP;
      return xtype;
    }
    case REALSXP: {
      if (Rf_inherits(x, "Date")) return CPP20_REALDATESXP;
      if (Rf_inherits(x, "POSIXct") && Rf_inherits(x, "integer64")) return CPP20_INT64PSXTSXP;
      if (Rf_inherits(x, "POSIXct")) return CPP20_REALPSXTSXP;
      if (Rf_inherits(x, "integer64")) return CPP20_INT64SXP; 
      return xtype;
    }
    // case VECSXP: {
    //   if (Rf_inherits(x, "data.frame")) return CPP20_DFSXP;
    //   return xtype;
    // }
    default: {
      return xtype;
    }
  }
}

inline const char* r_type_to_str(SEXPTYPE x){

  switch (x){
    case CPP20_INT64SXP: return "CPP20_INT64SXP";
    case CPP20_INTDATESXP: return "CPP20_INTDATESXP";
    case CPP20_REALDATESXP: return "CPP20_REALDATESXP";
    case CPP20_INT64PSXTSXP: return "CPP20_INT64PSXTSXP";
    case CPP20_REALPSXTSXP: return "CPP20_REALPSXTSXP";
    case CPP20_FCTSXP: return "CPP20_FCTSXP";
    case CPP20_DFSXP: return "CPP20_DFSXP";
    default: return Rf_type2char(x);
  }
}

template<typename T, typename U>
inline constexpr bool between_impl(const T x, const U lo, const U hi) {
  return x >= lo && x <= hi;
}

// Wrap any callable f, and return a new callable that:
//   - takes (auto&&... args)
//   - calls f(args...) inside cpp11::unwind_protect

// Like cpp11::safe but works also  for variadic fns
template <typename F>
auto r_safe_impl(F f) {
  return [f](auto&&... args)
    -> decltype(f(std::forward<decltype(args)>(args)...)) {

      using result_t = decltype(f(std::forward<decltype(args)>(args)...));

      if constexpr (std::is_void_v<result_t>) {
        cpp11::unwind_protect([&] {
          f(std::forward<decltype(args)>(args)...);
        });
        // no return; result_t is void
      } else {
        return cpp11::unwind_protect([&]() -> result_t {
          return f(std::forward<decltype(args)>(args)...);
        });
      }
    };

}

#define r_safe(F)                                                                      \
internal::r_safe_impl(                                                                 \
  [&](auto&&... args)                                                                  \
    -> decltype(F(std::forward<decltype(args)>(args)...)) {                            \
      return F(std::forward<decltype(args)>(args)...);                                 \
    }                                                                                  \
)


// If we find out eager initialisation of R symbols is a problem, we can use the constants below
// // Generic Lazy Loader for R Constants
// // Ptr: A pointer to the global R variable (e.g., &R_NilValue)
// template <typename T, T* Ptr>
// struct lazy_r_constant {
//     // Implicit conversion to SEXP (or whatever T is)
//     operator T() const noexcept {
//         return *Ptr;
//     }
    
//     // Allow comparison directly
//     bool operator==(T other) const noexcept { return *Ptr == other; }
//     bool operator!=(T other) const noexcept { return *Ptr != other; }
// };

// // lazy constants (to be defined later)
// inline constexpr lazy_r_constant<SEXP, &R_BlankString> blank_string_constant{};
// inline constexpr lazy_r_constant<SEXP, &R_MissingArg> missing_arg_constant{};

}

// Set & get the number of OMP threads
inline int get_threads(){
  auto n_threads = std::min(internal::cpp20_n_threads, OMP_MAX_THREADS);
  return n_threads > 1 ? n_threads : 1;
}
inline void set_threads(int n){
  int max_threads = OMP_MAX_THREADS;
  internal::cpp20_n_threads = std::min(n, max_threads);
}


// [[noexcept]] inline bool xor_(bool a, bool b){
//   return (a + b) == 1;
// }

namespace internal {

inline int calc_threads(r_size_t data_size){
  return data_size >= CPP20_OMP_THRESHOLD ? get_threads() : 1;
}

}

// Recycle loop indices
template<typename T>
inline constexpr void recycle_index(T& v, const T size) {
  v = (++v == size) ? T(0) : v;
}

template <typename... Args>
[[noreturn]] inline void abort(const char *fmt, Args... args){
  cpp11::stop(fmt, args...);
}

} // end of cpp20 namespace

#endif
