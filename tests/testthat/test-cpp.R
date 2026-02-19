test_that("Correct registration of cpp fns to R", {
  cpp_set_threads(2)
  expect_equal(cpp_get_threads(), 2)
  cpp_set_threads(1)
  expect_equal(cpp_get_threads(), 1)


  expect_equal(scalar1(10), 10)
  expect_equal(scalar2(10), 10)

  expect_equal(vector1(c(11, 12)), 11)
  expect_equal(vector2(c(11, 12)), 12)

  expect_equal(scalar3(12L, 10L), 22)

  expect_equal(scalar4(12L, 10.5), (12 + 10.5))

  expect_equal(test_sexp(1:3), 1:3)


  expect_equal(scalar_vec1(c(1, 2), 3), c(4, 5))

  # Since scalar_vec1 expects both inputs to be the same exact type, this should error
  expect_error(scalar_vec1(1:2, 3))

  expect_equal(scalar_vec2(1:2, 3), c(4, 5))

  expect_equal(scalar_vec3(1:2, 3L, 4, 5), c(13, 14))

  expect_equal(test_mix2(1,2,3,4,5,6,7), sum(1:7))

  expect_identical(test_specialisation(1:3), 1L)
  expect_identical(test_specialisation(c(1, 2, 3)), 0)


  expect_equal(test_str1("hi"), "hi")
  expect_equal(test_str2("hi"), "hi")
  expect_equal(test_str3("hi"), "hi")
  expect_equal(test_str4("hi"), "hi")

  expect_equal(test_as_sym("hi"), as.symbol("hi"))

  expect_identical(test_sexp2(mean), test_sexp2(mean))
  expect_error(test_sexp3(mean)) # Expects a list
  expect_identical(test_sexp3(list(mean)), list(mean))

  # Templated dispatch of SEXP/r_sexp
  # Should act as identity()
  expect_identical(test_sexp4(mean), mean)
  expect_identical(test_sexp4(1:3), 1:3)
  expect_identical(test_sexp4(1L), 1L)
  expect_identical(test_sexp4(list(1:10)), list(1:10))

  expect_identical(test_rval_identity(3), 3)
  expect_type(test_rval_identity("a"), "char") # Expects r_str (CHARSXP) and returns r_str (CHARSXP)
  expect_identical(test_rval_identity(as.symbol("a")), as.symbol("a"))
  expect_error(test_rval_identity(1:3))

  expect_identical(test_rval_identity(list(1L)), list(1L)) # Not sure about this
  # expect_error(test_rval_identity(list(1:3))) # Should definitely error!

  expect_identical(test_identity(1L), 1L)
  # expect_identical(test_identity(1:3), 1:3) # Should not error!

  expect_identical(test_list_to_scalars(list(0)), list(FALSE, 0L, 0, "0", list(0), as.symbol("0")))


  # expect_identical(test_as_int(1:10), 1:10)
  # expect_identical(test_as_int(as.double(1:10)), 1:10)
  # expect_identical(test_as_int(letters), rep(NA_integer_, 26))
  # expect_error(test_as_int(list(1)))
})
