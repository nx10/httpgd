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

#test_that("Append CSS with extra_css", {
#  testcss <- ".httpgd polyline { stroke: green; }"
#  hgd(webserver=F, extra_css = testcss)
#  plot(1)
#  svg <- hgd_svg()
#  dev.off()
#  expect_true(grepl(testcss, svg, fixed = TRUE))
#})