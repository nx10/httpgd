
#' Plot a test pattern that can be used to evaluate and compare graphics 
#' devices.
#'
#' @importFrom grDevices as.raster 
#' @importFrom graphics lines plot.new plot.window points polygon rasterImage
#'   rect segments text xinch yinch
#' @export
#'
#' @examples
#' \dontrun{
#'
#' hgd_test_pattern()
#' }
hgd_test_pattern <- function() {
  plot.new()
  plot.window(xlim = c(0, 100), ylim = c(0, 100))
  
  bx <- c(0, 100, 0, 100)
  by <- c(0, 0, 100, 100)
  
  points(bx, by, pch = 3)
  text(8 + bx * 0.84, by, sprintf("%i,%i", bx, by))
  text(50,
       100,
       "httpgd test graphics 0.1",
       cex = 0.8,
       col = "darkgray")
  
  rect(5, 5, 25, 20, border = "blue", col = "lightblue")
  text(5, 25, "rect")
  
  polygon(30 + c(0, 10, 15, 2, 8),
          22 + c(0, 0, 10, 5, 2),
          border = "darkgreen",
          col = "aquamarine")
  text(35, 35, "polygon")
  
  x <- 1:6
  points(30 + x * 3, 5 + (x %% 2) * 5)
  lines(30 + x * 3, 5 + (x %% 2) * 5, col = "red")
  text(35, 15, "polyline")
  
  points(15, 36, col = "chocolate")
  points(
    15,
    36,
    pch = 21,
    cex = 5,
    col = "darkviolet",
    bg = "coral"
  )
  text(15, 45, "circle")
  
  image <- as.raster(matrix(rep(0:1, length.out = 15), ncol = 5, nrow = 3))
  rasterImage(image, 60, 30, 65, 35, interpolate = FALSE)
  rasterImage(image, 60, 40, 65, 45)
  rasterImage(image, 70, 25, 70 + xinch(.5), 25 + yinch(.3),
              interpolate = FALSE)
  rasterImage(image,
              70,
              40,
              75,
              45,
              angle = 15,
              interpolate = FALSE)
  
  image <- as.raster(matrix(seq(0, 1, 1 / 3), ncol = 2))
  
  rasterImage(image, 60, 10, 70, 20, interpolate = FALSE)
  
  text(65, 55, "raster")
  
  text(10, 90, "text")
  text(5, 78, expression(bar(x) == sum(frac(x[i], n), i == 1, n)))
  
  text(26, 87, "A", cex = 0.5)
  text(30, 87, "A")
  text(34, 87, "A", cex = 1.5)
  
  text(6,
       68 - 1:4 * 4,
       c("plain", "bold", "italic", "bold-italic"),
       font = 1:4)
  
  text(32, 74, "\u00E9\u00E8 \u00F8\u00D8 \u00E5\u00C5 \u00E6\u00C6")
  
  text(25, 80, "0", srt = 0)
  text(30, 80, "25", srt = 25)
  text(35, 80, "90", srt = 90)
  text(40, 80, "180", srt = 180)
  
  points(20,
         62,
         pch = 3,
         cex = 4,
         col = "red")
  text(20, 62, "UR", adj = c(0, 0))
  text(20, 62, "UL", adj = c(1, 0))
  text(20, 62, "BR", adj = c(0, 1))
  text(20, 62, "BL", adj = c(1, 1))
  
  points(32,
         62,
         pch = 3,
         cex = 4,
         col = "red")
  text(32, 62, "B", adj = c(0.5, 1))
  text(32, 62, "R", adj = c(0, 0.5))
  text(32, 62, "L", adj = c(1, 0.5))
  text(32, 62, "U", adj = c(0.5, 0))
  
  points(44,
         62,
         pch = 3,
         cex = 4,
         col = "red")
  text(44, 62, "C")
  
  text(70, 90, "lines")
  
  segments(60 + 2.5 * 4, 85, 60 + 0:5 * 4, 70, lty = 0:6)
  points(60 + 0:5 * 4, rep(70, 6))
  
  segments(85, 90 - 0:4*5, 90, 90 - 0:4*5, lwd = 1:5*3)
  
  sapply(0:2, function(x)
    lines(
      rep(60 + x * 8, 3) + 0:2 * 2 ,
      c(60, 63, 60),
      lwd = 8,
      lend = x,
      ljoin = x
    ))
  
  points(rep(98, 26), 90 - 0:25 * 3, pch = 0:25, bg = "red")
}
