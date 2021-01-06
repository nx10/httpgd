# httpgd equivalent to svglite::xmlSVG
xmlSVG <- function(code, ...) {
  xml2::read_xml(hgd_inline(code, ...))
}

# Helper
# https://github.com/r-lib/svglite/blob/master/tests/testthat/helper-style.R

style_attr <- function(nodes, attr) {
  style <- xml2::xml_attr(nodes, "style")
  ifelse(
    grepl(sprintf("%s: [^;]*;", attr), style),
    gsub(sprintf(".*%s: ([^;]*);.*", attr), "\\1", style),
    NA_character_
  )
}

#mini_plot <- function(...) graphics::plot(..., axes = FALSE, xlab = "", ylab = "")

dash_array <- function(...) {
  x <- xmlSVG(mini_plot(1:3, ..., type = "l"))
  dash <- style_attr(xml2::xml_find_first(x, "//d1:polyline"), "stroke-dasharray")
  as.numeric(strsplit(dash, ",")[[1]])
}