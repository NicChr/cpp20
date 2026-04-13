
generate_cpp_regular_example <- function(){
  stop_unless_installed("usethis")
  proj_path <- usethis::proj_get()
  utils::getFromNamespace("use_src", "usethis")()
  cpp_path <- file.path(proj_path, "src", "code.cpp")
  brio::write_lines(c(
    "#include <cpp20.hpp>",
    "using namespace cpp20;",
    "
// Simple example showing how to register a C++ function to R
// You can compile and register this via `cpp20::cpp_source('src/code.cpp')`
// or via `cpp20::load_all()`

// For more info on cpp20 see https://nicchr.github.io/cpp20/index.html
    ",
    '
[[cpp20::register]]
r_dbl sum_doubles(r_vec<r_dbl> x){
  r_size_t n = x.length();
  r_dbl out(0);

  for (r_size_t i = 0; i < n; ++i){
    out += x.get(i);
  }
  return out;
}
    '),
cpp_path
  )
}


generate_cpp_template_example <- function(){
  stop_unless_installed("usethis")
  proj_path <- usethis::proj_get()
  utils::getFromNamespace("use_src", "usethis")()
  h_path <- file.path(proj_path, "src", "code.h")
  brio::write_lines(c(
    "#include <cpp20.hpp>",
    "using namespace cpp20;",
    "
// Simple example showing how to register a C++ function to R
// You can compile and register this via `cpp20::load_all()`

// For more info on cpp20 see https://nicchr.github.io/cpp20/index.html
    ",
    '
template <RMathType T>
[[cpp20::register]]
r_dbl sum_any(r_vec<T> x){
  r_size_t n = x.length();
  r_dbl out(0);

  for (r_size_t i = 0; i < n; ++i){
    out += x.get(i);
  }
  return out;
}
    '),
    h_path
  )
}
