#ifndef CPP20_R_MAKE_VEC_H
#define CPP20_R_MAKE_VEC_H


#include <cpp20/internal/r_vec.h>
#include <cpp20/internal/r_coerce.h>
#include <variant> // For variant

namespace cpp20 {

// Named argument
struct arg {
  const char* name;
  using storage_t = std::variant<
    std::monostate,
    r_lgl,
    r_int,
    r_int64,
    r_dbl,
    r_str,
    r_str_view,
    r_cplx,
    r_raw,
    r_sexp
  >;

  storage_t storage;

  // Default: no value yet
  explicit arg(const char* n)
    : name(n), storage(std::monostate{}) {}

  template <typename U>
  arg(const char* n, U&& v)
    : name(n), storage(as_r_val(std::forward<U>(v))) {}

  template <typename U>
  arg operator=(U&& v) const {
    return arg{name, std::forward<U>(v)};
  }
};


template <RVal T>
inline T as(const arg& a) {
  return std::visit(
    [](auto const& x) -> T {
      using X = std::remove_cvref_t<decltype(x)>;
      if constexpr (is<X, std::monostate>) {
        return T();
      } else {
        return as<T>(x);
      }
    },
    a.storage
  );
}


template<RVal T, typename... Args>
inline r_vec<T> make_vec(Args... args) {

  constexpr int n = sizeof...(args);

  if constexpr (n == 0){
    return r_vec<T>(0);
  } else {

    auto out = r_vec<T>(n);

    // Are any args named?
    constexpr bool any_named = (is<Args, arg> || ...);

    auto nms = any_named ? r_vec<r_str_view>(n) : r_vec<r_str_view>(r_null);

    int i = 0;
    (([&]() {
      if constexpr (is<Args, arg>) {
        out.set(i, as<T>(args));
        nms.set(i, as<r_str_view>(args.name));
      } else {
        out.set(i, as<T>(args));
      }
      ++i;
    }()), ...);

    attr::set_old_names(out.sexp, nms);
    return out;
  }
}

namespace attr {

template<typename... Args>
inline void modify_attrs(r_sexp x, Args... args) {
  auto attrs = make_vec<r_sexp>(args...);
  internal::modify_attrs_impl(x, attrs);
}

}

}

#endif
