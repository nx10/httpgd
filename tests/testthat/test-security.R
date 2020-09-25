test_that("User defined token", {
  testtok <- "123abc"
  httpgd(webserver=F, token=testtok)
  hs <- httpgdState()
  dev.off()
  expect_equal(hs$token, testtok)
})

test_that("Token R seed independence", {
  set.seed(1234)
  a <- httpgdGenerateToken(8)
  set.seed(1234)
  b <- httpgdGenerateToken(8)
  expect_false(isTRUE(all.equal(a, b)))
})