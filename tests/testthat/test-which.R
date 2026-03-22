test_that("which", {
  x <- c(TRUE, NA, TRUE, TRUE, FALSE, FALSE, NA, NA, TRUE, NA)

  expect_identical(test_which(x), which(x))
  expect_identical(test_which_inverted(x), which(!x %in% TRUE))

})
