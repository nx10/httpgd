test_that("Create pages", {
  httpgd(webserver=F)
  pnum <- 10
  for (i in 1:pnum) {
    plot.new()
  }
  hs <- httpgdState()
  dev.off()
  expect_equal(hs$hsize, pnum)
})

test_that("Delete pages", {
  httpgd(webserver=F)
  pnum <- 10
  dnum <- 3
  for (i in 1:pnum) {
    plot.new()
  }
  for (i in 1:dnum) {
    httpgdRemove()
  }
  hs <- httpgdState()
  dev.off()
  expect_equal(hs$hsize, pnum-dnum)
})

test_that("Get page by index", {
  httpgd(webserver=F)
  pnum <- 10
  dnum <- 3
  for (i in 1:pnum) {
    plot.new()
    teststr <- paste0("123abc_plot_", i)
    text(0, 0, teststr)
  }
  svg <- httpgdSVG(page = 4)
  dev.off()
  expect_true(grepl("123abc_plot_4", svg, fixed = TRUE))
})

test_that("Delete page by index", {
  httpgd(webserver=F)
  pnum <- 10
  dnum <- 3
  for (i in 1:pnum) {
    plot.new()
    teststr <- paste0("123abc_plot_", i)
    text(0, 0, teststr)
  }
  httpgdRemove(page = 4)
  hs <- httpgdState()
  svgs <- rep(NA, hs$hsize)
  for (i in 1:hs$hsize) {
    svgs[i] <- httpgdSVG(page = i)
  }
  dev.off()
  expect_true(grepl("123abc_plot_3", svgs[3], fixed = TRUE))
  expect_true(grepl("123abc_plot_5", svgs[4], fixed = TRUE))
})

test_that("Clear pages", {
  httpgd(webserver=F)
  pnum <- 10
  for (i in 1:pnum) {
    plot.new()
  }
  httpgdClear()
  hs <- httpgdState()
  dev.off()
  expect_equal(hs$hsize, 0)
})