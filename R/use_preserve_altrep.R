#' Adds the CPP20_PRESERVE_ALTREP flag to Makevars
#'
#' @description
#' Adds a flag to Makevars which enables lazy materialisation of
#' ALTREP vectors.
#'
#' @returns
#' Invisibly adds the CPP20_PRESERVE_ALTREP flag to Makevars.
#'
#' @export
use_preserve_altrep_flag <- function(){
  add_makevars_flag("PKG_CPPFLAGS", "-DCPP20_PRESERVE_ALTREP")
  cli::cli_bullets(c("v" = "Added CPP20_PRESERVE_ALTREP flag."))
}
