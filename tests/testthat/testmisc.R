context("Misc tests")

#test_that("Server startup", {
#  httpgd(port = 21378, token = 8, silent = TRUE)
#  tdev = dev.cur()
#  expect_equal(httpgdState()$port, 21378)
#  expect_equal(nchar(httpgdState()$token), 8)
#  dev.off(which = tdev)
#})

test_that("Random token R seed", {
  set.seed(1234)
  a <- httpgd_random_token(8)
  set.seed(1234)
  b <- httpgd_random_token(8)
  expect_false(isTRUE(all.equal(a, b)))
})
