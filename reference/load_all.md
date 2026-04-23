# A wrapper around `devtools::load_all()` specifically for cppally

A wrapper around
[`devtools::load_all()`](https://devtools.r-lib.org/reference/load_all.html)
specifically for cppally

## Usage

``` r
load_all(path = ".", debug = FALSE, ...)
```

## Arguments

- path:

  Path to package.

- debug:

  Should package be built without optimisations? Default is `FALSE`
  which builds with optimisations.

- ...:

  Further arguments passed on to
  [`pkgload::load_all()`](https://pkgload.r-lib.org/reference/load_all.html)

## Value

Invisibly registers cppally tagged functions and compiles C++ code.
