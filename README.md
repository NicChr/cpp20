
<!-- README.md is generated from README.Rmd. Please edit that file -->

# cpp20

<!-- badges: start -->

[![CRAN
status\|104](https://www.r-pkg.org/badges/version/cpp20)](https://CRAN.R-project.org/package=cpp20)

<!-- badges: end -->

**This is a work in progress!**

cpp20 is a high-performance header-only library providing a rich C++20
API for advanced R data manipulation. Leveraging C++20 Concepts, custom
R-based classes, templated functions and
Single-Instruction-Multiple-Data (SIMD) vectorisation, cpp20 enables
type-safety, performance, flexible templates and readable code.

#### Key cpp20 features

- Custom R-based C++ classes for scalars and vectors

- R-specific `NA` aware methods for the custom classes

- R-specific C++20 Concepts for template programming

- Automatic R object protection via a custom double-linked chunked
  vector list protection pool

- Registering C++ functions (including templated ones) to R

- Powerful coercion through `as<>`

- Seamless arithmetic and math operators

- Common vector manipulation methods provided as member functions

- Hybrid sorting using ska sort, counting sort, etc

- Fast hashing and grouping

- Fast summary stats

- Attribute manipulation

- Vectorised sequence creation

**Upcoming features**

- Combining vectors

- Common-type casting

- Data frames

### C++ types

cpp20 offers a rich set of R types in C++ that are NA-aware. This means
that common arithmetic and logical operations will account for `NA` in a
similar fashion to R.

#### r_lgl

cpp20’s scalar version of `logical`, `r_lgl` can represent true, false
or NA.

``` cpp
r_lgl r_true
r_lgl r_false
r_lgl r_na

// Alternatively

r_lgl true_value(true);
r_lgl false_value(false);
r_lgl na<r_lgl>();
```

Logical operators work just like in R

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

void lgl_ops(){
  r_true || r_false // true
  r_true && r_false // false
  r_na || r_true    // true
  r_na && r_true    // NA
  r_na && r_false   // false
  r_na || r_na      // NA
  r_na && r_na      // NA
}
```

**Using `r_lgl`** in if-statements

For type-safety reasons `r_lgl` cannot be implicitly converted to `bool`
except in if-statements. When an `r_lgl` is cast to a `bool`, the value
is checked to be `NA` and throws an error if it is. `NA` values must be
handled separately just like in R.

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

r_lgl condition = r_true;

void lgls(){

  // true is printed
  
  if (condition){
    print("true");
  } else {
    print("false");
  }
  
  r_lgl condition = r_na;
  
  // DONT DO THIS - an error is thrown!
  if (condition){
    print("true");
  } else {
    print("false");
  }
  
  // DO THIS instead
  if (is_na(condition)){
    print("na")
  } else if (condition){
    print("true");
  } else {
    print("false");
  }
  
  // Or alternatively without accounting for NA
  if (condition.is_true()){ // identical to R `isTRUE`
    print("true");
  } else {
    print("not true");
  }

}
```

All cpp20 scalar types are implemented as structs that contain the
underlying C/C++ types as well as other member functions.

| cpp20 type | Description | Underlying C/C++ Type | Implicitly converts to |
|:---|----|:---|:---|
| `r_lgl` | Scalar logical | `int` | `bool` **only** in if-statements |
| `r_int` | Scalar integer | `int` | `int` |
| `r_int64` | Scalar 64-bit integer | `int64_t` | `int64_t` |
| `r_dbl` | Scalar double | `double` | `double` |
| `r_str` | Scalar string | `r_sexp` | `SEXP` |
| `r_cplx` | Scalar double complex | `std::complex<double>` | `std::complex<double>` |
| `r_raw` | Scalar raw | `unsigned char` | `unsigned char` |
| `r_sym` | Symbol | `SEXP` | `SEXP` |
| `r_date` [^1] | Scalar date | `double` | `double` |
| `r_psxct` [^2] | Scalar date-time | double | `double` |
| `r_sexp` | Generic R object (SEXP)[^3] | `SEXP` | `SEXP` |

`NA` values can be accessed via the template function `na<T>`

#### C++ NA values and their R C API equivalents

| Type | Value | R C API Value | constexpr?[^4] |
|----|----|----|----|
| `r_lgl` | `na<r_lgl>()`/`r_na` | `NA_LOGICAL` | Yes |
| `r_int` | `na<r_int>()` | `NA_INTEGER` | Yes |
| `r_int64` | `na<r_int64>()` | Not applicable | Yes |
| `r_dbl` | `na<r_dbl>()` | `NA_REAL` | Yes |
| `r_str` | `na<r_str>()` | `NA_STRING` | No |
| `r_cplx` | `na<r_cplx>()` | Not applicable | Yes |
| `r_sym` | Not applicable | Not applicable | No |
| `r_sexp`[^5]<br><br><br> | `na<r_sexp>`/`r_null` | `R_NilValue` | No |

To access the underlying values and types consistently, use `unwrap()`
and `unwrap_t<>`. It is recommended to only use these when you
absolutely need to access the underlying types/values.

`unwrap()` returns the underlying C/C++ value - for example, `unwrap(x)`
where `x` is an `r_int` returns the underlying `int`. `unwrap_t<r_int>`
returns `int`.

Example usage

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

r_int x{10};
unwrap_t<r_int> y{unwrap(x)};
static_assert(is<decltype(y), int>, "y is expected to be of type int");

return x + y; // answer = 20
```

### Design choices

**Templates**

cpp20 makes heavy use of templates for powerful object-oriented
programming. While this offers a flexible framework for writing generic
functions, it comes at the cost of slower compile times and larger
binary sizes.

Users can write and optionally register their own templates (to R).
There are two main limitations to be aware of - a C++ specific one and
an R specific one. The C++ limitation is that templates generally must
be written in header files if they are to be used across multiple
compilation units. The R limitation has to do with automatic template
argument deduction. There is a workaround that I will discuss in a later
section but it is not ideal.

**Scalar R types and custom methods**

As shown in one of the previous sections, cpp20 offers R-based C++
scalar types that are `NA` aware. To achieve this multiple methods such
as binary arithmetic operators have been written to ensure `NA` is
propagated correctly. While every attempt has been made to make this as
fast as possible, it adds some overhead and in some cases can prevent
effective vectorisation (via e.g. SIMD instructions). If you find that
this is slowing things down too much you can work with the underlying
C/C++ types using `unwrap_t<>` and `unwrap()`.

**Automatic protection**

Like the excellent cpp11 package, cpp20 also handles automatic
protection for R objects.

Heavily inspired by cpp11’s double-linked protection list, we also use a
double-linked list system. Instead of a single pairlist, we use
double-linked chain of vector list chunks. We find that this generally
offers lower protection overhead. Furthermore, reference counting is
utilised to make copying `r_sexp` cheaper by avoiding re-inserting
copies into the protection list where possible.

The same caveats surrounding R longjmp errors that apply to cpp11
protection also applies to cpp20.

**R String views**

To avoid the overhead associated with automatic protection entirely, one
can use `r_str_view`, a non-owning class for R strings. `r_str_view` is
designed for read-only and short-lived contexts such as accessing and
manipulating the elements of a character vector. Using `r_str_view`
guarantees that no extra re-protections occur when the string object is
copied or moved due to the fact that it is simply a light wrapper around
`SEXP`. Similar to `std::string_view`, the major caveat is that you must
ensure the `r_str_view` object’s lifetime does not extend beyond the
lifetime of the R string (CHARSXP) it is pointing to. More on this
later.

**No copy-on-write or copy-on-modify**

Deep copies are almost never triggered when modifying vectors, a design
choice that contrasts cpp11’s copy-on-write approach. cpp20’s `r_vec<T>`
member `set()` always modifies in-place. It is up to the user to ensure
that a fresh vector is created before further manipulation or that it’s
safe to modify the existing vector.

**Differences between R and cpp20**

Any coercion that results in complete information loss is an error
(partial is allowed, e.g. double -\> int). This means that for example
`as<r_int>(r_str("a"))` will throw an error instead of returning an NA
like R does with `as.integer("a")`

The benefit of this is that when registering C++ functions to R, inputs
can be supplied flexibly without unexpected behaviour. Let’s say you
have a function foo that expects an `r_int` but you give it an `r_dbl`
without realising - this will implicitly coerce to `r_int` without
throwing an error. If it can’t be coerced to integer without complete
information loss (i.e non-parse-able string to integer), then an
informative error is thrown. If we allowed implicit coercion to NA then
we would need to be strict with inputs to balance things out.

**Vector indexing**

Most indexing is 0-based except when dealing with vectors of indices,
which are 1-indexed. 1-indexed indices are used in `subset()`, `find()`,
and other functions which accept or return a vector of indices.

**64-bit integers**

On the C++ side, 64-bit integers are fully supported, including vectors.
To return 64-bit integers to R we need the bit64 package to be loaded.
cpp20 delegates the handling of 64-bit integer vectors to bit64 by
marking them with the “integer64” class.

``` cpp
[[cpp20::register]]
r_int64 as_int64(r_int x){
    return as<r_int64>(x);
}
```

Please note that other 64-bit signed integer types like `int64_t`,
`R_xlen_t`, or cpp20’s identical `r_size_t` will convert to 64-bit
integer vectors when returned to R.

``` cpp
[[cpp20::register]]
r_size_t as_r_size_t(r_int x){
    return as<r_size_t>(x);
}
```

``` r
library(bit64)
as_int64(10L)
integer64
[1] 10
as_r_size_t(10L)
integer64
[1] 10
```

### Vectors

cpp20 vectors are templated and can be thought of as containers of the
scalar elements we talked about previously like `r_int`, `r_dbl`, etc.

We can create vectors like so

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

// Integer vector of size n
r_vec<r_int> new_integer_vector(int n){
  r_vec<r_int> int_vctr(n);
  return int_vctr;
}

// Usage
r_vec<r_int> new_filled_integer_vector(int n, r_int fill_value){
  r_vec<r_int> out(n, fill_value); // Fills all values with fill_value
  return out;
}

// You can also use fill() instead
r_vec<r_int> new_filled_integer_vector2(int n, r_int fill_value){
  r_vec<r_int> out(n);
  out.fill(0, r_int(n), fill_value); // Fill n-elements from 0 with fill_value
  return out;
}
```

### inline vectors

To create inline vectors, use `make_vec<>`

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

r_vec<r_dbl> foo(){
  return make_vec<r_dbl>(1, 1.5, 2, na<r_dbl>());
}
```

We can add names on the fly with `arg()`

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

r_vec<r_dbl> bar(){
  return make_vec<r_dbl>(
    arg("first") = 1, 
    arg("second") = 1.5, 
    arg("third") = 2, 
    arg("last") = na<r_dbl>()
  );
}
```

In R a list is a generic vector, so cpp20 defines lists as
`r_vec<r_sexp>`, a vector of the generic type `r_sexp`.

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

r_vec<r_sexp> cpp_list(){
  return make_vec<r_sexp>(1, 2, 3); // list(1, 2, 3)
}
```

Here is how to create a list of all cpp20 vectors of length 0

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

r_vec<r_sexp> all_vectors(){
  return make_vec<r_sexp>(
    arg("logical") = r_vec<r_lgl>(),
    arg("integer") = r_vec<r_int>(),
    arg("integer64") = r_vec<r_int64>(), // Requires bit64
    arg("double") = r_vec<r_dbl>(),
    arg("character") = r_vec<r_str>(),
    arg("character") = r_vec<r_str_view>(),
    arg("raw") = r_vec<r_raw>(),
    arg("date") = r_vec<r_date>(),
    arg("date-time") = r_vec<r_psxct>(),
    arg("list") = r_vec<r_sexp>()
  );
}
```

**Using `NULL`**

To avoid the use of additional meta-programming tactics to deal with
`NULL`, we instead allow vectors to hold `NULL` which makes programming
with R attributes easier. This means `r_vec<T>` objects can be
`R_NilValue`. To detect this, use the `is_null()` member function.

### Concepts

One of the most powerful features of C++20 are concepts. These allow
users to write human-readable templates and constraints. For example
r_math.h uses the `RMathType` concept for custom math functions to
ensure that only types that support math (like `r_dbl`) are allowed as
arguments to those functions. This promotes type-safety and signals to
the user what types are allowed.

Let’s look at an example in r_math.h

``` cpp
template <RMathType T>
inline constexpr T abs(T x){
  return is_na(x) ? x : T{internal::cpp_abs(unwrap(x))};
}
```

The top-line `template <RMathType T>` declares a template that
encapsulates `T`, an `RMathType`. The input is `x` is of type `T`, an
`RMathType` and the return value is also `T`

`T{}` is then used to construct the `RMathType` object from the absolute
result.

Some common concepts include

- RIntegerType - Includes `r_lgl`, `r_int`, `r_int64`

- RMathType - Includes `r_lgl`, `r_int`, `r_int64` and `r_dbl`

- RStringType - Includes `r_str` and `r_str_view`

- RScalar - Includes all cpp20 specific scalar types

- RVal - Includes anything a cpp20 vector (`r_vec<>`) can contain:
  RScalar +`r_sexp`

- RVector - Includes `r_vec<T>` where `T` is an RVal

### Coercion

To coerce from one scalar to another we can use `as<T>`

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

// Coerces NA correctly
r_int double_to_int(r_dbl x){
  return as<r_int>(x);
}

// Example usage
// double_to_int(r_dbl(1.5)) 
// double_to_int(na<r_dbl>())
```

We can also coerce from one vector type to another

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

// Coerces NA correctly
r_vec<r_int> to_int_vec(r_vec<r_dbl> x){
  return as<r_vec<r_int>>(x);
}

// Example usage
// to_int_vec(make_vec<r_dbl>(1, 1.5, na<r_dbl>()))
```

Since `as<T>` is extremely flexible, we can also coerce from a scalar to
a vector or vice versa, as well as to other types like `r_sexp`.

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

r_vec<r_sexp> coercions(){
  r_dbl a(4.2);
  r_vec<r_dbl> b(3, 0); // Length 3 double-vector filled with 0
  
  return make_vec<r_sexp>(
    as<r_vec<r_int>>(a),
    as<r_int>(b),
    as<r_str>(a),
    as<r_str>(b),
    as<r_sexp>(a),
    as<r_sexp>(b)
  );
}
```

### Strings

cpp20 provides two string types, `r_str` and `r_str_view`.

We can create R strings easily

``` cpp
r_str hello_str("hello");
```

To get a C or C++ string, use the members `c_str()` and `cpp_str()`
respectively

``` cpp
hello_str.c_str() // C-style string
hello_str.cpp_str() // C++ style string
```

### Symbols

Symbols are assumed to always be protected and so don’t have the same
overhead issues that `r_str` has. We can easily create symbols using the
`const char*` constructor

``` cpp
r_sym hello_sym("hello");
```

### Lists and views

`r_sexp` is generally interpreted as an “element of a list” since lists
are defined as `r_vec<r_sexp>`, a vector that holds generic `r_sexp`
elements. We can use `visit_vector` to disambiguate the type, i.e. from
an `r_sexp` to an `r_vec<r_int>` in the case that the element is an
integer vector. This must be used in a C++ lambda context.

**Example:** using `visit_vector()` to calculate list lengths

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

[[cpp20::register]]

r_vec<r_int> cpp_lengths(const r_vec<r_sexp>& x){
  r_size_t n = x.length();
  r_vec<r_int> out(n); // Initialise lengths vector
    for (r_size_t i = 0; i < n; ++i){
       visit_vector(x.view(i), [&](const auto& vec) {
         out.set(i, as<r_int>(vec.length()));
    });
    }
  return out;
}
```

**visit_sexp**

This allows one to visit more types than just vectors, including
factors, symbols and (soon to implemented) data frames. When an object’s
type can’t be deduced into a distinct type, `r_sexp` is returned.

**Example:** Using `visit_sexp()` to calculate list lengths

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

[[cpp20::register]]
r_vec<r_int> cpp_lengths2(const r_vec<r_sexp>& x){
    r_size_t n = x.length();
    r_vec<r_int> out(n); // Initialise lengths vector
      for (r_size_t i = 0; i < n; ++i){
         visit_sexp(x.view(i), [&](const auto& vec) {
         using vec_t = decltype(vec);
         if constexpr (!RVector<vec_t>){
             abort("Input must be an RVector");
         } else {
             out.set(i, as<r_int>(vec.length()));
         }
      });
      }
    return out;
}
```

### Factors

We can create a factor via `r_factors()`

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

[[cpp20::register]]
r_factors new_factor(r_vec<r_str> x){
    return r_factors(x);
}
```

``` r
 new_factor(letters)
 a b c d e f g h i j k l m n o p q r s t u v w x y z
Levels: a b c d e f g h i j k l m n o p q r s t u v w x y z
```

In cpp20, like R, factors are not vectors and therefore do not satisfy
the RVector concept. To access the underlying integer codes vector, use
the public `codes()` member function

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

static_assert(!RVector<r_factors>);

[[cpp20::register]]
r_vec<r_int> factor_codes(r_factors x){
    return x.codes();
}
```

``` r
letter_fct <- new_factor(letters)

letter_fct |> 
    factor_codes()
1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
```

### Attributes

Attributes can be manipulated via functions defined in the attr
namespace.

- `get_attrs()` - Returns a list of attributes (possibly
  `r_vec<r_sexp>(r_null)`)
- `set_attrs()` - Sets attributes to ones specified. Note: replaces any
  current attributes
- `clear_attrs()` - Removes all attributes
- `set_attr()` - Set a single attribute
- `get_attr()` - Get a single attribute
- `inherits1()` - Does object inherit class?
- `inherits_any()` - Does object inherit at least one of the specified
  classes?
- `inherits_all()` - Does object inherit all of the specified classes?
- `modify_attrs()` - Modifies current attributes but doesn’t remove any
  existing ones

### Sugar functions

cpp20 also offers many useful and high-performance common functions in
cpp20/sugar

- `n_unique()` - Fast number of unique values. Faster than
  `unique(x).length()` as it doesn’t need to allocate a unique vector

- unique - Like R’s `unique()` but with a `sort` argument to return
  sorted unique values

- identical - A very fast identical function that works for scalars and
  vectors. Use this for exact equality of any scalar or vector.

- match - Like R’s match, but also faster

- sequences - Like `sequence()` but it returns a list of sequences and
  also works with doubles.

- `order()` - Like base R’s order but it internally uses a hybrid
  approach of ska sort, count sorting, quick sort, etc.

- make_groups - An advanced function that returns a struct containing
  group IDs and number of groups (i.e number of unique group IDs). The
  `groups` struct contains the following members:

  - r_vec<r_int> ids - The cached group IDs
  - int n_groups - Number of unique groups
  - bool ordered - Do the group IDs specify a sorting order, or are they
    by order-of-first-appearance?
  - bool sorted - Are the group IDs sorted? (This can also be true for
    order-of-first-appearance IDs)
  - r_vec<r_int> start() - Returns an r_vec<r_int>(n_groups) vector of
    start locations of each unique group, signifying the location in the
    data at which each group initially appeared
  - r_vec<r_int> counts() - Returns an r_vec<r_int>(n_groups) vector of
    frequency counts of each unique group
  - r_vec<r_int> order() - Returns an r_vec<r_int>(ids.length()) order
    vector. This is a 0-indexed permutation vector that can be used to
    return sorted group IDs

- `recycle()` - Recycles supplied vectors to common length

- `r_vec<T>::subset()` - Fast subsetting of vectors

**Scalar math functions**

There are a rich suite of math functions. Some examples include `min()`,
`max()`, `round()`, `log()`, `floor()`, `ceiling()` and more.

**Stats sugar functions**

Some statistical summary functions that are all very highly optimised
for speed - `sum()` - Sum of values - `range()` - Min and max range of
values - `abs()` - Computes absolute values (there is also a scalar
version) - `var()` and `sd()` - Variance and standard deviation -
`gcd()` - Greatest common divisor - `lcm()` - Lowest common multiple

### templates

Templates must be defined in header files instead of cpp files. While
most templates can be registered to R, explicit instantiation (from R)
is impossible.

``` cpp
template <RScalar T>
[[cpp20::register]]
T scalar_init(){
    return T();
}
```

In C++ you would call this template like so

``` cpp
scalar_init<r_int>();
```

In R we naturally can’t do this so one alternative is to define a
prototype argument to allow argument deduction from that

``` cpp
template <typename T>
[[cpp20::register]]
T scalar_init(T ptype){
    return T();
}
```

And then in R

``` r
lapply(iris, scalar_init)

$Sepal.Length
numeric(0)

$Sepal.Width
numeric(0)

$Petal.Length
numeric(0)

$Petal.Width
numeric(0)

$Species
factor()
Levels:
```

Generally speaking most templates won’t need explicit instantiation and
we can rely on automatic deduction

``` cpp
template <RVector T>
[[cpp20::register]]
int cpp_length(T vec){
    return vec.length();
}
```

``` r
cpp_length(letters)
[1] 26
```

`RVector` is a concept that includes all R atomic vectors (including
64-bit integer) and lists. As it was specified as the template argument
type, if we supply an argument that doesn’t satisfy RVector (i.e isn’t a
vector), then cpp20’s R dispatch code will catch this and return an
error.

``` r
cpp_length(iris$Species)

Error: No matching template instantiation found for input types
```

For non-template functions the input type must be specified

``` cpp
[[cpp20::register]]
r_dbl add_half(r_dbl x){
  return x + 0.5;
}
```

``` r
add_half(1.5) # Double
[1] 2
```

We can even supply an integer (`r_int`) because the integer is coerced
to a double (`r_dbl`) internally via `as<r_dbl>`

``` r
add_half(1L) # Integer works too
[1] 1.5
```

The internal generated code looks like this in src/cpp20.cpp

``` cpp

// readme.h
r_dbl add_half(r_dbl x);
extern "C" SEXP _cpp20_add_half(SEXP x) {
  BEGIN_CPP20
  return cpp_to_sexp(add_half(as<r_dbl>(x)));
  END_CPP20
}
```

Notice the `as<r_dbl>(x)`, this is the line that coerces our integer to
a double correctly

What happens if we supply an input that can’t be coerced to an `r_dbl`?

``` cpp
add_half("A")

Error: Implicit NA coercion detected from r_str to r_dbl, please ensure data can be coerced without complete loss of information
```

Thankfully due to the way cpp20 is strict about completely lossy
coercions, an informative error is thrown.

**Symbols**

`r_sym` is unsupported in templates when it’s part of a template
argument but is supported when the argument is explicitly an `r_sym`.

``` cpp
[[cpp20::register]]
r_str symbol_to_string(r_sym x){
    return as<r_str>(x);
}
```

``` r
hello_world_symbol <- as.symbol("hello world!")
hello_world_symbol
`hello world!`
symbol_to_string(hello_world_symbol)
[1] "hello world!"
```

### Registering R functions

To make a C++ function available to R we use the `[[cpp20::register]]`
tag.

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

[[cpp20::register]]
void hello_world(){
  print("Hello World!");
}
```

Now the function is available in R

``` r
> hello_world()
Hello World!
```

### Registering templates

To register a C++ template to R, one must declare the
`[[cpp20::register]]` target after the template declaration.

``` cpp
#include <cpp20.hpp>
using namespace cpp20;

template <RStringType T>
[[cpp20::register]]
void my_print(T x){
  print(x.c_str());
}
```

**views**

As briefly mentioned earlier, views can be used to eliminate the small
overhead associated with automatic protection of objects wrapping SEXP.
`r_str_view` is one such class which is a lightweight around a `SEXP`
and never protects the underlying `SEXP`. Its lifetime must be shorter
than the object it is pointing to.

**DO**:

``` cpp
void good(r_str x){
    r_str_view str = x;
    if (str.cpp_str() == "true"){
    print("true");
    } else {
    print("false");
    }
}
```

**DON’T**:

``` cpp
r_str_view bar(){
    r_str new_str("I will be destroyed at the end of `bar()`");
    
    r_str_view bad_str = new_str; // A view of new_str
    return bad_str; // Points to underlying CHARSXP but nothing protecting it
}
```

The above is a classic example of what **not** to do with string views,
and that is have the view outlive the owner. In this case `bad_str` is
returned at the end of `bar()` at which point `new_str` goes out of
scope and gets destroyed. This means nothing is protecting the
underlying `CHARSXP` that `new_str` was protecting and once that
`CHARSXP` is garbage-collected by R, `bad_str` will become a
dangling-pointer and you will get a memory leak.

All core cpp20 concepts

- RIntegerType - Includes `r_lgl`, `r_int`, `r_int64`

- RMathType - Includes `r_lgl`, `r_int`, `r_int64` and `r_dbl`

- RStringType - Includes `r_str` and `r_str_view`

- RScalar - Includes all cpp20 specific scalar types

- RVal - Includes anything a cpp20 vector (`r_vec<>`) can contain:
  RScalar +`r_sexp`

- RVector - Includes `r_vec<T>` where `T` is an RVal

- RTimeType - Includes `r_date` and `r_psxct`

- RNumericType - Numeric types, including RMathType and RTimeType

- RSortableType - Includes RNumericType and RStringType (strings can
  also be sorted)

- RAtomicVector - A vector that contains RScalar elements

- RCpp20Type - Any R type defined by R, including RVal, RVector,
  RFactor, RDataFrame, RSymbol

- CppType - Anything that is not an RCpp20Type

- CastableToRScalar - Anything that can be constructed or cast into an
  RScalar (which also includes RScalar)

- CastableToRVal (**questioning**) - Anything that can be constructed or
  cast into an RVal. This is more complicated as it includes vectors,
  factors and data frames which can be cast to `r_sexp`

Other useful type traits

- `unwrap_t` - Returns the underlying unwrapped type
- `as_r_scalar_t` - Returns the equivalent RScalar type
- `as_r_val_t` - Returns the equivalent RVal type
- `common_r_t` - Returns the common RVal type between 2 types. Generally
  this is a hierarchy where the common type is the type that both values
  can be coerced to without complete loss of information

[^1]: Unlike `r_str` which is composite and holds an `r_sexp` member,
    `r_date` and `r_psxct` instead inherit directly from `r_dbl`. This
    means that they can implicitly convert to `r_dbl`

[^2]: Unlike `r_str` which is composite and holds an `r_sexp` member,
    `r_date` and `r_psxct` instead inherit directly from `r_dbl`. This
    means that they can implicitly convert to `r_dbl`

[^3]: `r_sexp` represents a generic R object which can include cpp20
    vectors. We will explain how to disambiguate `r_sexp` later which is
    most useful when working with lists and data frames

[^4]: In C++ constexpr is used as a keyword to declare that it’s
    possible to evaluate values at compile-time, meaning they are known
    before any code is run by the user. Since `r_na` internally is the
    largest possible `int` which does not change and is known a priori,
    it is therefore a compile-time constant.

[^5]: Having an `NA` sentinel for `r_sexp` is very useful when writing
    templates involving vectors. For this reason the `NA` sentinel is
    `r_null`. This doesn’t mean `is_na(r_null)` is true, and is
    intentionally not true because it is not a scalar and therefore
    cannot be `NA`. As `r_null` represents the absence of a tangible R
    object, it can be thought of as a zero-length object and since all
    `NA` values are represented as length-1 vectors (in R),
    `is_na(r_null)` should not return true. \#### Accessing the
    underlying types and values
