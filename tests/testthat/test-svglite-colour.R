test_that("transparent blacks are written", {
  x <- xmlSVG({
    plot.new()
    points(0.5, 0.5, col = rgb(0, 0, 0, 0.25))
    points(0.5, 0.5, col = rgb(0, 0, 0, 0.50))
    points(0.5, 0.5, col = rgb(0, 0, 0, 0.75))
  })

  circle <- xml2::xml_find_all(x, ".//d1:circle")

  expect_equal(style_attr(circle, "stroke"), rep("#000000", 3))
  expect_equal(style_attr(circle, "stroke-opacity"), c("0.25", "0.50", "0.75"))
})

test_that("transparent colours are not written", {
  x <- xmlSVG({
    plot.new()
    points(0.5, 0.5, col = NA)
  })

  circle <- xml2::xml_find_all(x, ".//d1:circle")
  expect_length(circle, 0)
})