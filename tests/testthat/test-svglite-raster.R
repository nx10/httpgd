
test_that("raster exists", {
  x <- xmlSVG({
    image(matrix(runif(64), nrow = 8), useRaster = TRUE)
  })
  ns <- xml2::xml_ns(x)

  img <- xml2::xml_attr(xml2::xml_find_all(x, ".//d1:image", ns = ns), "xlink:href", ns = ns)
  expect_gt(nchar(img), 1000)
})