
cpp_decorations <- function(pkg = ".", files = decor::cpp_files(pkg = pkg), is_attribute = TRUE) {

  cpp_attribute_pattern <- function(is_attribute){
    paste0("^[[:blank:]]*", if (!is_attribute)
      "//[[:blank:]]*", "\\[\\[", "[[:space:]]*(.*?)[[:space:]]*",
      "\\]\\]", "[[:space:]]*")
  }

  res <- lapply(files, function(file) {
    if (!file.exists(file)) {
      return(tibble(file = character(), line = integer(),
                    decoration = character(), params = list(), context = list()))
    }

    lines <- if (is_attribute) read_lines(file) else readLines(file)

    start <- grep(cpp_attribute_pattern(is_attribute), lines)
    if (!length(start)) {
      return(tibble(file = character(), line = integer(),
                    decoration = character(), params = list(), context = list()))
    }

    end <- c(tail(start, -1L) - 1L, length(lines))

    # Adjust 'start' to include preceding 'template <...>' line(s)
    # This effectively pulls the template definition into the 'context'
    # so parse_cpp_function sees it

    real_start <- start
    real_end <- end
    for (i in seq_along(start)) {
      idx <- start[i]
      # Walk back to find template
      curr <- idx - 1L
      while(curr > 0) {
        line <- lines[curr]
        if (grepl("^\\s*//", line)) { # Skip comments
          curr <- curr - 1
          next
        }
        if (is_template_signature(line)) {
          real_start[i] <- curr
          n_steps_back <- idx - curr
          # Adjust previous end point
          if (i > 1){
            real_end[i - 1] <- real_end[i - 1] - n_steps_back
          }
          break
        }

        if (is_requires_signature(line)){
          curr <- curr - 1L
          next
        }

        if (nzchar(trimws(line))) {
          break
        }
        curr <- curr - 1L
      }
    }

    text <- lines[start]
    content <- sub(paste0(cpp_attribute_pattern(is_attribute), ".*"), "\\1", text)
    decoration <- sub("\\(.*$", "", content)
    has_args <- grepl("\\(", content)
    params <- map_if(content, has_args, function(.x) {
      set_names(as.list(parse(text = .x)[[1]][-1]))
    })

    # Context uses real start/end
    context <- map2(real_start, real_end, \(.x, .y) lines[seq(.x, .y)])

    # Remove empty characters (at the end)
    n_empty <- integer(length(context))
    for (j in seq_along(context)){
      fn_line <- context[[j]]
      n_empty[j] <- 0L
      for (i in rev(seq_len(length(fn_line)))){
        if (nzchar(fn_line[i])){
          break
        }
        n_empty[j] <- n_empty[j] + 1L
      }
    }
    context <- map2(context, n_empty, \(x, y) x[seq_len(length(x) - y)])

    tibble(file, line = start, decoration, params, context)
  })

  vctrs::vec_rbind(!!!res)
}
