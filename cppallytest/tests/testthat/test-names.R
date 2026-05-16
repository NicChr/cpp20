test_that("names are mutated in-place across wrappers around the same SEXP", {
  expect_identical(test_names_inplace_mutation(), "b")
})

test_that("a pre-built hash is invalidated by set_names", {
  expect_true(test_names_stale_invalidation())
})

test_that("attr::set_attr(x, names_sym, ...) invalidates the cache", {
  expect_true(test_names_set_attr_invalidation())
})

test_that("hash table grows correctly past the initial reserve", {
  expect_true(test_names_growing())
})

test_that("registry sweep handles many short-lived wrappers", {
  expect_true(test_names_sweep())
})

test_that("shallow_copy + set_names does not poison the source's view", {
  expect_true(test_names_shallow_copy_isolation())
})

test_that("empty names vector lookups return NA cleanly", {
  expect_true(test_names_empty())
})

test_that("every key is findable after a grow, on repeated passes", {
  expect_true(test_names_roundtrip_after_grow())
})
