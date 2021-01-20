test_that("par(cex) affects strwidth", {
  xmlSVG({
    plot.new()
    w1 <- strwidth("X")
    par(cex = 4)
    w4 <- strwidth("X")
  })
  expect_equal(w4 / w1, 4, tol = 1e-3)
})

test_that("cex affects strwidth", {
  hgd_inline(height = 7 * 72, width = 7 * 72, {
    plot.new()
    w1 <- strwidth("X")
    w4 <- strwidth("X", cex = 4)
  })
  expect_equal(w4 / w1, 4, tol = 1e-3)
})

test_that("special characters are escaped", {
  x <- xmlSVG({
    plot.new()
    text(0.5, 0.5, "<&>")
  })
  # xml_text unescapes for us - this still tests that the
  # file parses, which it wouldn't otherwise
  expect_equal(xml2::xml_text(xml2::xml_find_first(x, ".//d1:text")), "<&>")
})

test_that("utf-8 characters are preserved", {
  skip_on_os("windows") # skip because of xml2 buglet
  skip_if_not(l10n_info()$`UTF-8`)

  x <- xmlSVG({
    plot.new()
    text(0.5, 0.5, "\u00b5")
  })
  # xml_text unescapes for us - this still tests that the
  # file parses, which it wouldn't otherwise
  expect_equal(xml2::xml_text(xml2::xml_find_first(x, ".//d1:text")), "\u00b5")
})

test_that("special characters are escaped", {
  x <- xmlSVG({
    plot.new()
    text(0.5, 0.5, "a", col = "#113399")
  })
  # xml_text unescapes for us - this still tests that the
  # file parses, which it wouldn't otherwise
  expect_equal(style_attr(xml2::xml_find_first(x, ".//d1:text"), "fill"), "#113399")
})

test_that("default point size is 12", {
  x <- xmlSVG({
    plot.new()
    text(0.5, 0.5, "a")
  })
  expect_equal(style_attr(xml2::xml_find_first(x, ".//d1:text"), "font-size"), "12.00px")
})

test_that("cex generates fractional font sizes", {
  x <- xmlSVG({
    plot.new()
    text(0.5, 0.5, "a", cex = 0.1)
  })
  expect_equal(style_attr(xml2::xml_find_first(x, ".//d1:text"), "font-size"), "1.20px")
})

test_that("a symbol has width greater than 0", {
  xmlSVG({
    plot.new()
    strw <- strwidth(expression(symbol("\042")))
  })
  expect_lt(.Machine$double.eps, strw)
})


# manual test
#test_that("strwidth and height correctly computed", {
#  hgd(width=4 * 72, height=4 * 72, user_fonts = fontquiver::font_families("Bitstream Vera"))
#
#  plot.new()
#  str <- "This is a string"
#  text(0.5, 0.5, str)
#
#  h <- strheight(str)
#  w <- strwidth(str)
#
#  rect(0.5 - w / 2, 0.5 - h / 2, 0.5 + w / 2, 0.5 + h / 2)
#})

test_that("strwidth has fallback for unknown glyphs", {
  xmlSVG(user_fonts = fontquiver::font_families("Bitstream Vera"), {
    plot.new()
    w <- strwidth("正規分布")
  })
  expect_true(w > 0)
})