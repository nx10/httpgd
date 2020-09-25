test_that("SVG contains plot text", {
  httpgd(webserver=F)
  plot.new()
  teststr <- "Some text abc123"
  text(0, 0, teststr)
  svg <- httpgdSVG()
  dev.off()
  expect_true(grepl(teststr, svg, fixed = TRUE))
})