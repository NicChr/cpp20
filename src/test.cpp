#include <cpp20.hpp>

using namespace cpp20;

[[cpp11::register]]
void dummy() {
}


[[cpp11::register]]
void cpp_set_threads(int n){
  set_threads(n);
}


[[cpp11::register]]
SEXP foo(SEXP x) {
  // auto ok = internal::as_r_string(r_true);
  // return as_vector(as<r_string_t>(r_true));
  // return r_vector_t<r_lgl>(x);
// return as<r_vector_t<r_string_t>>(r_vector_t<r_lgl>(x));

  SEXP out = internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return as<r_vec<r_str_view>>(xvec);
  });
  return out;

  // return as<r_vector_t<r_string_t>>(r_vector_t<r_lgl>(x));

  // // return make_list(as<r_string_t>(1.5), as<r_string_t>(na::real));
  // return as_vector(as<r_string_t>(vec::new_vector<r_complex_t>(1, 123)));
}

[[cpp11::register]]
SEXP bar(SEXP x) {
  SEXP out = internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return as<r_vec<r_int>>(xvec);
  });
  return out;
}


[[cpp11::register]]
SEXP foobar(SEXP x) {
    auto out = internal::visit_vector(x, [&](auto xvec) -> r_vec<r_lgl> {
      return xvec.is_na();
  });
  return out;
}



[[cpp11::register]]
SEXP foofoo() {
  auto out = r_posixcts(10);
  out.fill(0, out.length(), 0);
  out.set_tzone(as<r_str_view>("UTC"));

  return out;
}


[[cpp11::register]]
SEXP foo6(SEXP x){
    auto out = internal::visit_vector(x, [&](auto xvec) -> r_posixcts {
    return as<r_posixcts>(xvec);
  });

  return out;
}


[[cpp11::register]]
SEXP foo7(SEXP x){
    auto out = internal::visit_vector(x, [&](auto xvec) -> r_vec<r_str_view> {
    return xvec.names();
  });

  return out;
}


[[cpp11::register]]
SEXP foo9(){
  auto x = make_vec<r_int>(1, 2, 3);
  auto y = make_vec<r_dbl>(arg("x") = 2.5);
  auto out = make_vec<r_sexp>(arg("first") = x, arg("second") = y);

  return out;
}

[[cpp11::register]]
SEXP foo_attrs(SEXP x){
  return attr::get_attrs(r_sexp(x));
}


[[cpp11::register]]
SEXP foo12(SEXP x, SEXP attrs){
  auto y = r_vec<r_int>(x);
  attr::set_attrs(y.sexp, as<r_vec<r_sexp>>(attrs));
  return y;
}

// Go from an r_sexp to an r_vec<>

[[cpp11::register]]
SEXP foo14(){
  r_sexp x = r_sexp(r_vec<r_int>(10));
  auto out = as<r_vec<r_str_view>>(x);

  return out;
}

// Compilation error (expected)
// SEXP foo19(SEXP x){
//   return as_vector(x);
// }


[[cpp11::register]]
SEXP foo26(){
  return make_vec<r_sexp>(arg("r_sexp") = sizeof(r_sexp), arg ("SEXP") = sizeof(SEXP), arg("sexp") = sizeof(cpp11::sexp), arg("r_str_view") = sizeof(r_str_view),
arg("r_dbl") = sizeof(r_dbl), arg("r_int") = sizeof(r_int), arg("r_lgl") = sizeof(r_lgl), arg("r_cplx") = sizeof(r_cplx),
  arg("r_sym") = sizeof(r_sym), arg("r_vec<r_int>") = sizeof(r_vec<r_int>));
}


[[cpp11::register]]
SEXP foo27(SEXP cols){
  return internal::new_df_impl(r_vec<r_sexp>(cols));
}

[[cpp11::register]]
SEXP foo28a(SEXP x, SEXP y){
  r_vec<r_sexp> x_ = r_vec<r_sexp>(x);
  r_sexp y_ = r_sexp(y);
  int n = x_.length();
  for (int i = 0; i < n; ++i){
    SET_VECTOR_ELT(x_, i, y);
  }
  return x_;
}

[[cpp11::register]]
SEXP foo28b(SEXP x, SEXP y){
  r_vec<r_sexp> x_ = r_vec<r_sexp>(x);
  r_sexp y_ = r_sexp(y);
  int n = x_.length();
  for (int i = 0; i < n; ++i){
    x_.set(i, y_);
  }
  return x_;
}

[[cpp11::register]]
int foo30(){
  r_lgl x = r_true;
  // return !x;
  if (x != r_false){
    return 1;
  } else {
    return 2;
  }
}


[[cpp11::register]]
SEXP foo_rep_len(SEXP x, int n) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return xvec.rep_len(n);
  });
}


[[cpp11::register]]
SEXP foo_sum(SEXP x, bool na_rm) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using T = typename decltype(xvec)::data_type;
    if constexpr (any<T, r_lgl, r_int, r_dbl>){
      return as_vector(sum(xvec, na_rm));
    } else {
      return r_null;
    }
  });
}
[[cpp11::register]]
SEXP foo_mean(SEXP x, bool na_rm) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using T = typename decltype(xvec)::data_type;
    if constexpr (RMathType<T>){
      return as_vector(mean(xvec, na_rm));
    } else {
      return r_null;
    }
  });
}
[[cpp11::register]]
SEXP foo_var(SEXP x, bool na_rm) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using T = typename decltype(xvec)::data_type;
    if constexpr (RMathType<T>){
      return as_vector(var(xvec, na_rm));
    } else {
      return r_null;
    }
  });
}
[[cpp11::register]]
SEXP foo_sum_int(SEXP x, bool na_rm) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using T = typename decltype(xvec)::data_type;
    if constexpr (any<T, r_int>){
      return as_vector(sum_int(xvec, na_rm));
    } else {
      return r_null;
    }
  });
}
[[cpp11::register]]
SEXP foo_range(SEXP x, bool na_rm) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using T = typename decltype(xvec)::data_type;
    if constexpr (any<T, r_int, r_dbl>){
      return as_vector(range(xvec, na_rm));
    } else {
      return r_null;
    }
  });
}

[[cpp11::register]]
SEXP foo_na_count(SEXP x) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return as<r_vec<r_int>>(xvec.na_count());
  });
}


[[cpp11::register]]
SEXP foo_is_na(SEXP x) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return xvec.is_na();
  });
}

[[cpp11::register]]
SEXP foo39(SEXP x, bool na_rm){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using t = decltype(xvec);
    if constexpr (is<t, r_vec<r_int>> || is<t, r_vec<r_dbl>> || is<t, r_vec<r_lgl>>){
      return range(xvec, na_rm);
    } else {
      return r_null;
    }
  });
}

[[cpp11::register]]
SEXP foo40(SEXP x, bool na_rm){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using t = decltype(xvec);
    if constexpr (is<t, r_vec<r_int>> || is<t, r_vec<r_dbl>> || is<t, r_vec<r_lgl>>){
      return as_vector(min(xvec, na_rm));
    } else {
      Rf_error("error");
      return r_null;
    }
  });
}

[[cpp11::register]]
SEXP foo41(SEXP x){
  r_vec<r_int> y = r_vec<r_int>(x);
  return as<r_vec<r_dbl>>(sum_int(y));
}


[[cpp11::register]]
SEXP foo42(SEXP x) {
  auto xvec = r_vec<r_dbl>(x);

    r_size_t n = xvec.length();

    r_dbl sum(0);

    for (r_size_t i = 0; i < n; ++i){
      sum += xvec.get(i);
    }
    return as_vector(sum);
}


[[cpp11::register]]
SEXP foo43(SEXP x) {
  auto xvec = r_vec<r_dbl>(x);

    r_size_t n = xvec.length();
    const auto *p_x = xvec.data();

    r_dbl sum(0);

    for (r_size_t i = 0; i < n; ++i){
      sum += p_x[i];
    }
    return as_vector(sum);
}


[[cpp11::register]]
SEXP foo44(SEXP x) {
  auto xvec = r_vec<r_dbl>(x);

    r_size_t n = xvec.length();
    const auto* RESTRICT p_x = xvec.data();

    r_dbl sum(0);

    for (r_size_t i = 0; i < n; ++i){
      sum += p_x[i];
    }
    return as_vector(sum);
  }


[[cpp11::register]]
SEXP foo48(SEXP x) {
  auto xvec = r_vec<r_int>(x);

    r_size_t n = xvec.length();

    r_int min_ = r_limits<r_int>::max();

    for (r_size_t i = 0; i < n; ++i){
      min_ = min(min_, xvec.get(i));
    }
    return as_vector(min_);
}

[[cpp11::register]]
SEXP foo49(SEXP x, bool na_rm){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using t = decltype(xvec);
    if constexpr (is<t, r_vec<r_int>> || is<t, r_vec<r_dbl>> || is<t, r_vec<r_lgl>>){
      return as_vector(sum(xvec, na_rm));
    } else {
      Rf_error("error");
      return r_null;
    }
  });
}

[[cpp11::register]]
SEXP foo50(SEXP x, bool na_rm){
  r_vec<r_int> x_ = r_vec<r_int>(x);

  return as<r_vec<r_dbl>>(sum_int(x_, na_rm));
}


[[cpp11::register]]
SEXP foo51(SEXP x){
  r_vec<r_int> x_ = r_vec<r_int>(x);
  return abs(x_);
}



[[cpp11::register]]
SEXP foo54(SEXP x){
  auto y = r_vec<r_int>(x);
  return as_r_val(y);
}


[[cpp11::register]]
SEXP foo_add1(SEXP x, SEXP y) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    auto yvec = as<decltype(xvec)>(y);
    using data_t = typename decltype(xvec)::data_type;
    if constexpr (any<data_t, r_lgl, r_int, r_int64, r_dbl>){
      return as_vector(xvec.get(0) + yvec.get(0));
    } else {
      return r_null;
    }
  });
}


[[cpp11::register]]
SEXP foo_add(SEXP x){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using data_t = typename decltype(xvec)::data_type;
    r_size_t n = xvec.length();
    if constexpr (RMathType<data_t>){
      auto out = r_vec<data_t>(n);
      OMP_SIMD
      for (r_size_t i = 0; i < n; ++i){
        out.set(i, xvec.get(i) + xvec.get(i));
      }
      return out;
    } else {
      return r_null;
    }
  });
}

[[cpp11::register]]
SEXP foo_vec_add(SEXP x, SEXP y){
    return internal::visit_vector(x, [=](auto xvec) -> SEXP {
      return internal::visit_vector(y, [=](auto yvec) -> SEXP {
        // 2. Use remove_cvref_t to ensure we have the raw type
        using FromType = typename std::remove_cvref_t<decltype(xvec)>::data_type;
        using ByType = typename std::remove_cvref_t<decltype(yvec)>::data_type;
        
        // 3. Check types
        if constexpr (RMathType<FromType> && RMathType<ByType>){
          return xvec + yvec;
        } else {
          return r_null;
        }
      });
    });
}

[[cpp11::register]]
SEXP foo_vec_subtract(SEXP x, SEXP y){
  auto xvec = as<r_vec<r_int>>(x);
  auto yvec = as<r_vec<r_int>>(y);
  return xvec - yvec;
}


[[cpp11::register]]
SEXP foo_vec_multiply(SEXP x, SEXP y){
  auto xvec = as<r_vec<r_int>>(x);
  auto yvec = as<r_vec<r_int>>(y);
  return xvec * yvec;
}

[[cpp11::register]]
SEXP foo_vec_divide(SEXP x, SEXP y){
  auto xvec = as<r_vec<r_int>>(x);
  auto yvec = as<r_vec<r_int>>(y);
  return xvec / yvec;
}

[[cpp11::register]]
SEXP foo_factor(SEXP x){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return r_factors(xvec);
  });
}
[[cpp11::register]]
SEXP foo_factor2(SEXP x, SEXP levels){
  return r_factors(as<r_vec<r_str_view>>(x), as<r_vec<r_str_view>>(levels));
}


[[cpp11::register]]
SEXP foo_gcd(SEXP x, bool na_rm){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using data_t = typename decltype(xvec)::data_type;
    if constexpr (RMathType<data_t>){
      return as_vector(gcd(xvec, na_rm));
    } else {
      return r_null;
    }
  });
}

[[cpp11::register]]
SEXP foo_lcm(SEXP x, bool na_rm){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using data_t = typename decltype(xvec)::data_type;
    if constexpr (RMathType<data_t>){
      return as_vector(lcm(xvec, na_rm));
    } else {
      return r_null;
    }
  });
}


[[cpp11::register]]
SEXP foo_sset(SEXP x, SEXP i){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return xvec.subset(as<r_vec<r_int>>(i));
  });
}


[[cpp11::register]]
SEXP foo_lgl(){
  r_lgl a = r_true;
  r_lgl b = r_false;
  r_lgl c = r_na;
  return make_vec<r_sexp>(
    a && a, // true
    a || a, // true
    b && b, // false
    b || b, // false
    a && b, // false
    a || b, // true
    c || a, // true
    c && b, // false
    a || c, // true
    b && c, // false
    a && c, // NA
    b && c, // false
    b || c, // NA
    c && c, // NA
    c || c // NA
  );
}

[[cpp11::register]]
SEXP foo_unique(SEXP x) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return unique(xvec);
  });
}

[[cpp11::register]]
SEXP foo_sorted_unique(SEXP x) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return sorted_unique(xvec);
  });
}

[[cpp11::register]]
SEXP foo_as_str(SEXP x) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    if constexpr (any<decltype(xvec), r_vec<r_sexp>>){
      return r_null;
    } else {
      return as<r_vec<r_str_view>>(xvec);
    }
  });
}


[[cpp11::register]]
SEXP foo_match(SEXP x, SEXP table) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return match(xvec, as<decltype(xvec)>(table));
  });
}


[[cpp11::register]]
SEXP foo_all_whole(SEXP x) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return as_vector(all_whole_numbers(xvec));
  });
}

[[cpp11::register]]
SEXP foo_copy(SEXP x) {
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return deep_copy(xvec);
  });
}

[[cpp11::register]]
SEXP foo_group_id(SEXP x){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    return make_groups(xvec).ids;
  });
}

[[cpp11::register]]
SEXP foo_n_groups(SEXP x){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using data_t = typename decltype(xvec)::data_type;
    if constexpr (is<data_t, r_sexp>){
      return r_null;
    } else {
      return as_vector(make_groups(xvec).n_groups);
    }
  });
}

[[cpp11::register]]
SEXP foo_groups_sorted(SEXP x){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using data_t = typename decltype(xvec)::data_type;
    if constexpr (is<data_t, r_sexp>){
      return r_null;
    } else {
      return as_vector(make_groups(xvec).sorted);
    }
  });
}

[[cpp11::register]]
SEXP foo_group_starts(SEXP x, int n_groups, bool sorted){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using data_t = typename decltype(xvec)::data_type;
    if constexpr (is<data_t, r_sexp>){
      return r_null;
    } else {
      auto y = r_vec<r_int>(x);
      groups ok(y, n_groups, sorted);
      return group_starts(ok);
    }
  });
}


[[cpp11::register]]
SEXP foo_remainder(){
  return make_vec<r_dbl>(
    r_dbl(5) % 2.0, 
    r_int(5) % 2, 
    5 % 2, 
    r_dbl(5) % r_int(2),
    5 % r_int(2),
    5 % r_dbl(2.2),
    5.0 % r_dbl(2.2),
    std::fmod(5, 2.2),
    -r_dbl(5) % r_dbl(2.2),
    r_dbl(5) % -r_dbl(2.2),
    r_int(20) % -r_int(3),
    -r_int(20) % r_int(3),
    r_dbl(20) % -r_dbl(3),
    r_int(1) % r_dbl(0.2),
    r_dbl(1) % r_dbl(0.2),
    r_limits<r_dbl>::max() % 2
  );
}


[[cpp11::register]]
SEXP foo_between(SEXP x, SEXP lo, SEXP hi){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using data_t = typename decltype(xvec)::data_type;
    if constexpr (any<data_t, r_int, r_dbl>){
      // return between(xvec, lo, hi);
      return between(xvec, as<decltype(xvec)>(lo), as<decltype(xvec)>(hi));
    } else {
      return r_null;
    }
  });
  
}

[[cpp11::register]]
SEXP foo_make_vec_test(){
  return make_vec<r_int>(make_vec<r_int>(1, 2, 3));
}

[[cpp11::register]]
SEXP foofoofoo(){
  return make_vec<r_lgl>(is_nan(r_dbl(R_NaN)), is_nan(r_dbl(NA_REAL)), is_nan(r_dbl(std::sqrt(-1.0))), is_nan(r_dbl(NA_REAL)));
}

[[cpp11::register]]
SEXP foo_which(SEXP x){
  return which(as<r_vec<r_lgl>>(x));
}


[[cpp11::register]]
SEXP foo_which_inverted(SEXP x){
  return which(as<r_vec<r_lgl>>(x), true);
}

[[cpp11::register]]
SEXP foo_asint64(SEXP x) {
  return as<r_vec<r_int64>>(x);
}

[[cpp11::register]]
SEXP foo_order(SEXP x){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using T = typename decltype(xvec)::data_type;
    if constexpr (RSortable<T>){
      return order(xvec);
    } else {
      return r_null;
    }
  });
}

[[cpp11::register]]
SEXP foo_stable_order(SEXP x){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using T = typename decltype(xvec)::data_type;
    if constexpr (RSortable<T>){
      return stable_order(xvec);
    } else {
      return r_null;
    }
  });
}
[[cpp11::register]]
SEXP foo_slow_order(SEXP x){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using T = typename decltype(xvec)::data_type;
    if constexpr (RSortable<T>){
      return internal::cpp_order(xvec);
    } else {
      return r_null;
    }
  });
}

[[cpp11::register]]
SEXP foo_group_id2(SEXP x){
  return internal::visit_vector(x, [&](auto xvec) -> SEXP {
    using T = typename decltype(xvec)::data_type;
    if constexpr (RSortable<T>){
      return internal::make_groups_from_order(xvec, order(xvec)).ids;
    } else {
      return foo_group_id(x);
    }
  });
}


[[cpp11::register]]
SEXP foo_sorted(SEXP x){
  return as<r_sexp>(is_sorted(as<r_vec<r_int>>(x)));
}

[[cpp11::register]]
void test1(){
  // internal::as_r<r_str_view>(r_null);
  // std::string_view x = "ok";
  // std::string y = x;
  // std::string_view z = y;

  // r_str_view a("ok");
  // r_str b = a;
  // r_str_view c = b;
}


[[cpp11::register]]
SEXP foo_str_vectors(SEXP x){
  auto a = r_vec<r_str>(x);
  auto b = as<r_vec<r_str_view>>(a);
  auto c = as<r_vec<r_str>>(b);
  return make_vec<r_sexp>(a, b, c);
}

[[cpp11::register]]
SEXP foo_seqs(SEXP sizes, SEXP from, SEXP by){
  return sequences(as<r_vec<r_int>>(sizes), as<r_vec<r_int>>(from), as<r_vec<r_int>>(by));
}
[[cpp11::register]]
SEXP foo_seqs2(SEXP sizes, SEXP from, SEXP by){
  // 1. Capture by value [=] to safely capture 'sizes' (SEXP pointer)
  return internal::visit_vector(from, [=](auto fromvec) -> SEXP {
    return internal::visit_vector(by, [=](auto byvec) -> SEXP {
      // 2. Use remove_cvref_t to ensure we have the raw type
      using FromType = typename std::remove_cvref_t<decltype(fromvec)>::data_type;
      using ByType = typename std::remove_cvref_t<decltype(byvec)>::data_type;
      
      // 3. Check types
      if constexpr (RMathType<FromType> && RMathType<ByType>){
        return sequences(as<r_vec<r_int>>(sizes), fromvec, byvec);
      } else {
        return r_null;
      }
    });
  });
}



[[cpp11::register]]
SEXP foo_test(){
  return as<r_sexp>(int(1));
}
