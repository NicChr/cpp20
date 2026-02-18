test_that("Correct registration of cpp to R", {
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



})
