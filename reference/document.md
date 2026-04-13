# A wrapper around `devtools::document()` to support cpp20 package development

A wrapper around
[`devtools::document()`](https://devtools.r-lib.org/reference/document.html)
to support cpp20 package development

## Usage

``` r
document(pkg = ".", roclets = NULL, quiet = FALSE)
```

## Arguments

- pkg:

  See
  [`?devtools::document`](https://devtools.r-lib.org/reference/document.html)

- roclets:

  See
  [`?devtools::document`](https://devtools.r-lib.org/reference/document.html)

- quiet:

  See
  [`?devtools::document`](https://devtools.r-lib.org/reference/document.html)

## Value

Invisibly updates roxygen documentation, compiles C++ code and exports
cpp20 tagged functions to R.
