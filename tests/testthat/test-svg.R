test_that("SVG contains plot text", {
  hgd(webserver=F)
  plot.new()
  teststr <- "Some text abc123"
  text(0, 0, teststr)
  svg <- hgd_svg()
  dev.off()
  expect_true(grepl(teststr, svg, fixed = TRUE))
})

test_that("boxplot returns valid SVG", {
  expect_warning(xmlSVG({
    boxplot(rnorm(10))
  }), regexp = NA)
})