test_that("User defined token", {
  testtok <- "123abc"
  hgd(webserver=F, token=testtok)
  hs <- hgd_state()
  dev.off()
  expect_equal(hs$token, testtok)
})

test_that("Token R seed independence", {
  set.seed(1234)
  a <- hgd_generate_token(8)
  set.seed(1234)
  b <- hgd_generate_token(8)
  expect_false(isTRUE(all.equal(a, b)))
})