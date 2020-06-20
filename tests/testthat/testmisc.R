context("Misc tests")

test_that("Server startup", {
  httpgd(port = 21378, token = 8, silent = TRUE)
  tdev = dev.cur()
  expect_equal(httpgdState()$port, 21378)
  expect_equal(nchar(httpgdState()$token), 8)
  dev.off(which = tdev)
})
