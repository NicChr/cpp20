add_makevars_omp_flag <- function(lines, var) {
  pattern <- paste0("^\\s*", var, "\\s*[+:]?=")
  idx <- grep(pattern, lines)
  if (length(idx) > 0) {
    if (!grepl("$(SHLIB_OPENMP_CXXFLAGS)", lines[idx[1]], fixed = TRUE)) {
      lines[idx[1]] <- paste(lines[idx[1]], "$(SHLIB_OPENMP_CXXFLAGS)")
    }
  } else {
    lines <- c(lines, paste(var, "=", "$(SHLIB_OPENMP_CXXFLAGS)"))
  }
  lines
}

#' Helper for developing packages with cpp20
#'
#' @description
#' usethis style helper to add the necessary setup to a new package to help
#' users get started with writing C++ code.
#'
#' @returns
#' Invisibly sets up the necessary conditions for
#' developing a package with cpp20.
#'
#' @export
use_cpp20 <- function (){
  stop_unless_installed(c("rlang", "usethis", "desc", "purrr"))
  proj_path <- utils::getFromNamespace("proj_path", "usethis")
  utils::getFromNamespace("check_is_package", "usethis")("use_cpp20()")
  rlang::check_installed("cpp20")
  utils::getFromNamespace("check_uses_roxygen", "usethis")("use_cpp20()")
  utils::getFromNamespace("check_has_package_doc", "usethis")("use_cpp20()")
  utils::getFromNamespace("use_src", "usethis")()
  utils::getFromNamespace("use_dependency", "usethis")("cpp20", "LinkingTo")
  desc <- desc::desc()
  desc$set(SystemRequirements = "C++20")
  desc$write()
  cat(
    paste0("useDynLib(", utils::getFromNamespace("project_name", "usethis")(), ", .registration = TRUE)\n"),
    file = proj_path("NAMESPACE"),
    append = TRUE
  )

  # Add OPENMP flags to Makevars
  makevars_path <- proj_path("src", "Makevars")
  if (file.exists(makevars_path)) {
    lines <- brio::read_lines(makevars_path)
  } else {
    lines <- character()
  }
  lines <- add_makevars_omp_flag(lines, "PKG_CXXFLAGS")
  lines <- add_makevars_omp_flag(lines, "PKG_LIBS")
  brio::write_lines(lines, makevars_path)

  # Generate code examples
  generate_cpp_regular_example()
  generate_cpp_template_example()

  cli::cli_inform(c(
    "Please run {.run cpp20::document()} to finish setup",
    "For continuous development please use {.run cpp20::load_all()} and {.run cpp20::document()}"
  ))

  invisible()
}
