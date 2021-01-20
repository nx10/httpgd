test_that("adds default background", {
  x <- xmlSVG(plot.new())
  expect_equal(style_attr(xml2::xml_find_first(x, "./d1:rect"), "fill"), "#FFFFFF")
})

test_that("adds background set by device driver", {
  x <- xmlSVG(plot.new(), bg = "red")
  expect_equal(style_attr(xml2::xml_find_first(x, "./d1:rect"), "fill"), rgb(1, 0, 0))
})

test_that("default background respects par", {
  x <- xmlSVG({
    par(bg = "red")
    plot.new()
  })
  expect_equal(style_attr(xml2::xml_find_first(x, "./d1:rect"), "fill"), rgb(1, 0, 0))
})

test_that("if bg is transparent in par(), use device driver background", {
  x <- xmlSVG({
    par(bg = NA)
    plot.new()
  }, bg = "blue")
  style <- xml2::xml_text(xml2::xml_find_first(x, "//d1:style"))
  expect_match(style, "fill: none;")
  expect_equal(style_attr(xml2::xml_find_first(x, "./d1:rect"), "fill"), rgb(0, 0, 1))
})

# not applicable:
#test_that("creating multiple pages is identical to creating multiple individual svgs", {
