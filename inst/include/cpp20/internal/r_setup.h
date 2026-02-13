#ifndef CPP20_R_SETUP_H
#define CPP20_R_SETUP_H

#include <cpp11.hpp>
#include <type_traits>
#include <ankerl/unordered_dense.h> // Unique + match
#include <cstring> // For strcmp
#include <complex>
#include <limits>
#include <vector>
#include <algorithm>

#ifdef _MSC_VER
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

#ifndef R_NO_REMAP
#define R_NO_REMAP 
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

inline constexpr int64_t CPP20_OMP_THRESHOLD = 100000;
inline constexpr SEXPTYPE CPP20_INT64SXP = 64;
inline int cpp20_n_threads = 1;

inline SEXPTYPE CPP20_TYPEOF(SEXP x){
  return Rf_inherits(x, "integer64") ? internal::CPP20_INT64SXP : TYPEOF(x);
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


namespace internal {

inline int calc_threads(r_size_t data_size){
  return data_size >= CPP20_OMP_THRESHOLD ? get_threads() : 1;
}

}

// Recycle loop indices
template<typename T>
inline constexpr void recycle_index(T& v, const T size) {
  v = (++v == size) ? static_cast<T>(0) : v;
}

template <typename... Args>
[[noreturn]] inline void abort(const char *fmt, Args... args){
  cpp11::stop(fmt, args...);
}

} // end of cpp20 namespace

// Making complex numbers hashable
namespace ankerl {
namespace unordered_dense {

template <>
struct hash<std::complex<double>> {
    using is_avalanching = void;

    [[nodiscard]] auto operator()(std::complex<double> const& x) const noexcept -> uint64_t {
        // Use ankerl's built-in hash for doubles
        auto h1 = hash<double>{}(x.real());
        auto h2 = hash<double>{}(x.imag());
        
        // We combine them using a mixing function
        return ankerl::unordered_dense::detail::wyhash::mix(h1, h2);
    }
};

} // namespace unordered_dense
} // namespace ankerl

#endif
