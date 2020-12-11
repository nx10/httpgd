
test_that("radius is not given in points", {
  x <- xmlSVG({
    plot.new()
    points(0.5, 0.5, cex = 20)
    text(0.5, 0.5, cex = 20)
  })
  circle <- xml2::xml_find_all(x, ".//d1:circle")
  expect_equal(xml2::xml_attr(circle, "r"), "54.00")
})

test_that("points are given stroke and fill", {
  x <- xmlSVG({
    plot.new()
    points(0.5, 0.5, pch = 21, col = "red", bg = "blue", cex = 20)
  })
  circle <- xml2::xml_find_all(x, ".//d1:circle")
  expect_equal(style_attr(circle, "stroke"), rgb(1, 0, 0))
  expect_equal(style_attr(circle, "fill"), rgb(0, 0, 1))
})

test_that("points get alpha stroke and fill given stroke and fill", {
  x <- xmlSVG({
    plot.new()
    points(0.5, 0.5, pch = 21, col = rgb(1, 0, 0, 0.1), bg = rgb(0, 0, 1, 0.1), cex = 20)
  })
  circle <- xml2::xml_find_all(x, ".//d1:circle")
  expect_equal(style_attr(circle, "stroke"), rgb(1, 0, 0))
  expect_equal(style_attr(circle, "stroke-opacity"), "0.10")
  expect_equal(style_attr(circle, "fill"), rgb(0, 0, 1))
  expect_equal(style_attr(circle, "fill-opacity"), "0.10")
})

test_that("points are given stroke and fill (none)", {
  x <- xmlSVG({
    plot.new()
    points(0.5, 0.5, pch = 21, col = "red", bg = NA, cex = 20)
  })
  style <- xml2::xml_text(xml2::xml_find_first(x, "//d1:style"))
  expect_match(style, "fill: none;")

  circle <- xml2::xml_find_all(x, ".//d1:circle")
  expect_equal(style_attr(circle, "fill"), NA_character_)
})