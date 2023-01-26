# https://github.com/r-lib/svglite/blob/master/tests/testthat/test-output.R

test_that("different string and file output produce identical svg", {
  hgd(webserver=F)

  ## 1. Write to a file
  f1 <- tempfile()
  on.exit(unlink(f1))
  plot(1:5)
  hgd_svg(file=f1)
  dev.off()

  out1 <- readLines(f1, warn = FALSE)

  ## 2. Write to a string stream
  hgd(webserver=F)
  plot(1:5)
  s <- hgd_svg()
  dev.off()
  out2 <- strsplit(s, "\n")[[1]]

  expect_equal(out1, out2)
})

test_that("intermediate outputs are always valid svg", {
  hgd(webserver=F)
  
  expect_valid_svg <- function() {
    expect_error(xml2::read_xml(hgd_svg()), NA)
  }

  mini_plot(1:10)
  expect_valid_svg()

  rect(2, 2, 3, 3)
  expect_valid_svg()

  segments(5, 5, 6, 6)
  expect_valid_svg()

  dev.off()
})



