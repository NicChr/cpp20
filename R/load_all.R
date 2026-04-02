# cd src
#
# g++ -std=gnu++20 -fopenmp -Wa,-mbig-obj -O2 -Wall -mfpmath=sse -msse2 -mstackrealign \
# -I"C:/PROGRA~1/R/R-45~1.2/include" -DNDEBUG -I../inst/include \
# -I"C:/rtools45/x86_64-w64-mingw32.static.posix/include" \
# -x c++-header -o cpp20_pch.h.gch cpp20_pch.h

# build_pch <- function(path = ".") {
#   pkg_path <- normalizePath(path)
#   src_dir  <- file.path(pkg_path, "src")
#   pch_h    <- file.path(src_dir, "cpp20_pch.h")
#   pch_gch  <- file.path(src_dir, "cpp20_pch.h.gch")
#
#   if (!file.exists(pch_h)) {
#     stop("src/cpp20_pch.h not found in package at ", pkg_path)
#   }
#
#   r_bin <- file.path(R.home("bin"), "R")
#   r_cfg <- function(var) {
#     raw <- suppressWarnings(system2(r_bin, c("CMD", "config", var), stdout = TRUE, stderr = FALSE))
#     if (!is.null(attr(raw, "status")) && attr(raw, "status") != 0L) return(character(0))
#     Filter(nzchar, strsplit(trimws(paste(raw, collapse = " ")), "\\s+")[[1]])
#   }
#
#   cxx  <- r_cfg("CXX20")[[1]]
#   omp_flag <- if (r_cfg("SHLIB_OPENMP_CXXFLAGS") |> length() > 0L) {
#     r_cfg("SHLIB_OPENMP_CXXFLAGS")
#   } else if (.Platform$OS.type == "windows") {
#     "-fopenmp"
#   } else {
#     character(0)
#   }
#   # Run from src/ and use the same relative -I../inst/include that Makevars uses,
#   # so GCC's include-path validation accepts the .gch when compiling source files.
#   args <- c(
#     r_cfg("CXX20STD"),
#     r_cfg("CXX20FLAGS"),
#     omp_flag,
#     r_cfg("CPPFLAGS"),
#     paste0("-I", R.home("include")),
#     "-I../inst/include",
#     if (.Platform$OS.type == "windows") "-Wa,-mbig-obj",
#     "-x", "c++-header",
#     "-o", "cpp20_pch.h.gch",
#     "cpp20_pch.h"
#   )
#
#   message("Building precompiled header...")
#   old_wd <- setwd(src_dir)
#   on.exit(setwd(old_wd), add = TRUE)
#
#   ret <- system2(cxx, args)
#   if (ret == 0L) {
#     message("Done: ", pch_gch)
#   } else {
#     warning("PCH build failed (exit code ", ret, ")")
#   }
#   invisible(ret == 0L)
# }

#' A wrapper around `devtools::load_all()` specifically for cpp20
#'
#' @param path Path to package.
#' @param debug Should package be built without optimisations?
#' Default is `FALSE` which builds with optimisations.
#' @param ... Further arguments passed on to `pkgload::load_all()`
load_all <- function (path = ".", debug = FALSE, ...){
  stop_unless_installed(c("pkgload", "rstudioapi"))
  if (inherits(path, "package")) {
    path <- path$path
  }
  if (rstudioapi::hasFun("documentSaveAll")) {
    rstudioapi::documentSaveAll()
  }
  pkgload::load_all(path = path, debug = debug, compile = FALSE, quiet = TRUE, helpers = FALSE)
  cpp_register(path = path)
  pkgload::load_all(path = path, debug = debug, ...)
}
