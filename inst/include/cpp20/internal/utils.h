#ifndef CPP20_UTILS_H
#define CPP20_UTILS_H

// cpp20 Core definitions and templates
// License: MIT

#include <cpp20/internal/r_setup.h>
#include <cpp20/internal/r_concepts.h>
#include <cpp20/internal/r_types.h>
#include <cpp20/internal/r_symbols.h>
#include <cpp20/internal/r_env.h>
#include <cpp20/internal/r_limits.h>
#include <cpp20/internal/r_nas.h>
#include <cpp20/internal/r_methods.h>
#include <cpp20/internal/r_rtype_coerce.h>
#include <cpp20/internal/r_vec.h>
#include <cpp20/internal/r_attrs.h>
#include <cpp20/internal/r_copy.h>
#include <cpp20/internal/r_factor.h>
#include <cpp20/internal/r_list.h>
#include <cpp20/internal/r_coerce.h>
#include <cpp20/internal/r_make_vec.h>
#include <cpp20/internal/r_df.h>
#include <cpp20/internal/r_exprs.h>
#include <cpp20/internal/r_fns.h>
#include <cpp20/internal/r_math.h>
#include <cpp20/internal/r_vec_math.h>
#include <cpp20/internal/r_stats.h>


namespace cpp20 {


// Compact seq generator as ALTREP, same as `seq_len()`
// inline r_vec<r_int> compact_seq_len(r_size_t n){
//   if (n < 0){
//     abort("`n` must be >= 0");
//   }
//   if (n == 0){
//     return r_vec<r_int>();
//   }
//   r_sexp colon_fn = fn::find_pkg_fun(":", "base", false);
//   r_sexp out = fn::eval_fn(colon_fn, env::base_env, 1, n);
//   return r_vec<r_int>(out);
// }


// We call R fn`cpp20::set_threads` to make sure the R option is set
inline void set_threads(uint16_t n){
  uint16_t max_threads = OMP_MAX_THREADS;
  uint16_t threads = std::min(n, max_threads);
  r_sexp cpp20_set_threads = fn::find_pkg_fun("set_threads", "cpp20", true);
  auto r_threads = as_vector(as<r_int>(threads));
  fn::eval_fn(cpp20_set_threads, env::base_env, r_threads);
}

} // End of cpp20 namespace

#endif
