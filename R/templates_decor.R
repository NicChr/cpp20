

# To be used on decor context
is_template_signature <- function(x){
  if (!is.character(x)){
    stop("`x` must be a character vector")
  }
  # Remove whitespace from start/end
  x <- trimws(x)

  # Match template<>
  template_pattern <- "^template\\s*\\<.*\\>$"
  grepl(template_pattern, x, perl = TRUE)
}

# To be used on decor context
is_requires_signature <- function(x){
  if (!is.character(x)){
    stop("`x` must be a character vector")
  }
  # Remove whitespace from start/end
  x <- trimws(x)

  # Match requires clause
  # The nice thing about c++ is that
  # it won't allow lines like: requires () {
  # or requires {}
  # which should separate it from being detected as a function
  template_pattern <- "^requires\\s*.+"
  grepl(template_pattern, x, perl = TRUE)
}

is_template_arg <- function(type, template_param) {
  pattern <- paste0("^", template_param, "$|<", template_param, "[,>]|,\\s*", template_param, "[,>]")
  grepl(pattern, type)
}

is_any_template_arg <- function(type, template_params) {
  for (param in template_params){
    if (is_template_arg(type, param)){
     return(TRUE)
    }
  }
  FALSE
}

# Once string has been confirmed to be a template signature
# extract the typenames
# template_typenames <- function(x){
#
#   if (!is_template_signature(x)){
#     cli_abort("{.arg x} must be a valid template signature of the form 'template <typename T>'")
#   }
#
#   text <- gsub("^template\\s*(\\<.*\\>)$", "\\1", x, perl = TRUE)
#
#   multiple_params <- str_detect(text, "\\<.+,.+\\>")
#
#
#   if (multiple_params){
#     # Typenames followed by a comma and preceded by '<
#     first_params <- str_extract(text, "(?<=\\<)(.+(?=,))")
#     type_strs <- c(
#       first_params,
#       # Typenames preceded by a comma and followed by >
#       str_extract_all(text, "(?<=,)(.+(?=\\>))")
#     )
#   } else {
#     # case - Only one param
#     type_strs <- gsub("\\<(.+)\\>", "\\1", text, perl = TRUE)
#   }
#   type_strs <- trimws(type_strs)
#
#   # Empty template
#   if (type_strs == "<>"){
#     return(tibble(type_names = character(), arg_names = character()))
#   }
#
#   # Separate out the type names from the arg names
#
#   # type_names <- regmatches(type_strs, m = regexpr(".+(?=(\\s+.+))", type_strs, perl = TRUE))
#
#   # Anything followed by whitespace
#   type_names <- regmatches(type_strs, m = regexpr(".+(?=\\s+)", type_strs, perl = TRUE))
#
#   # Match whitespace + anything
#   arg_names <- trimws(regmatches(type_strs, m = regexpr("\\s+.+", type_strs, perl = TRUE)))
#
#   if ((length(type_strs) != length(type_names)) || (length(type_strs) != length(type_names))){
#     cli_abort(c(
#       "{.arg x} must be a valid template signature of the form 'template <first_type T, second_type U, ...>'",
#        "not: {x}"
#     ))
#   }
#
#   tibble(template_type = type_names, type = arg_names)
# }

get_template_params <- function(context) {
  # Extract content between template < ... >
  # Handles "template <typename T, RVector U>"
  pattern <- "template\\s*<([^>]+)>"
  match <- regmatches(context, regexpr(pattern, context))

  if (length(match) == 0) return(character(0))

  # Remove 'template <' and '>'
  inner <- sub("template\\s*<", "", sub(">$", "", match))

  # Split by comma
  parts <- strsplit(inner, ",")[[1]]

  # Extract the last word of each part (the variable name)
  # e.g., "typename T" -> "T", "RVector U" -> "U"
  params <- trimws(sub(".*\\s+(\\w+)$", "\\1", parts))
  params
}

# adjust_context <- function(context){
#   which_template <- which(map_lgl(unname(context), is_template_signature))
#   template_str <- context[which_template]
# }

parse_cpp_function <- function(context, is_attribute = TRUE){

  # Remove lines containing template or requires clauses
  is_template_clause <- is_template_signature(context)
  is_requires_clause <- is_requires_signature(context)

  context <- context[!is_template_clause & !is_requires_clause]
  out <- decor::parse_cpp_function(context, is_attribute = is_attribute)
  attr(out, "cpp_template") <- any(is_template_clause)
  out

}


