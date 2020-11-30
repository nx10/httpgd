test_that("Create pages", {
  hgd(webserver=F)
  pnum <- 10
  for (i in 1:pnum) {
    plot.new()
  }
  hs <- hgd_state()
  dev.off()
  expect_equal(hs$hsize, pnum)
})

test_that("Delete pages", {
  hgd(webserver=F)
  pnum <- 10
  dnum <- 3
  for (i in 1:pnum) {
    plot.new()
  }
  for (i in 1:dnum) {
    hgd_remove()
  }
  hs <- hgd_state()
  dev.off()
  expect_equal(hs$hsize, pnum-dnum)
})

test_that("Get page by index", {
  hgd(webserver=F)
  pnum <- 10
  dnum <- 3
  for (i in 1:pnum) {
    plot.new()
    teststr <- paste0("123abc_plot_", i)
    text(0, 0, teststr)
  }
  svg <- hgd_svg(page = 4)
  dev.off()
  expect_true(grepl("123abc_plot_4", svg, fixed = TRUE))
})

test_that("Delete page by index", {
  hgd(webserver=F)
  pnum <- 10
  dnum <- 3
  for (i in 1:pnum) {
    plot.new()
    teststr <- paste0("123abc_plot_", i)
    text(0, 0, teststr)
  }
  hgd_remove(page = 4)
  hs <- hgd_state()
  svgs <- rep(NA, hs$hsize)
  for (i in 1:hs$hsize) {
    svgs[i] <- hgd_svg(page = i)
  }
  dev.off()
  expect_true(grepl("123abc_plot_3", svgs[3], fixed = TRUE))
  expect_true(grepl("123abc_plot_5", svgs[4], fixed = TRUE))
})

test_that("Clear pages", {
  hgd(webserver=F)
  pnum <- 10
  for (i in 1:pnum) {
    plot.new()
  }
  hgd_clear()
  hs <- hgd_state()
  dev.off()
  expect_equal(hs$hsize, 0)
})