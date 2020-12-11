# todo (both are manual)

#test_that("regression test for no clipping", {
#  svglite("test-no-clip.svg", 4, 4, user_fonts = bitstream)
#  on.exit(dev.off())

#  mini_plot(c(-1, 1), c(-1, 1), asp = 1, type = "n")
#  rect(-0.5, -0.5, 0.5, 0.5, col = "blue")
#  text(0, 0.5, "Clipping", cex = 2, srt = 30)
#  abline(h = 0.5, col = "red")
#})

#test_that("regression test for clipping", {
#  svglite("test-clip.svg", 4, 4, user_fonts = bitstream)
#  on.exit(dev.off())

#  mini_plot(c(-1, 1), c(-1, 1), asp = 1, type = "n")
#  clip(-1, 0, -1, 0)
#  rect(-0.5, -0.5, 0.5, 0.5, col = "blue")
#  clip(0, 1, 0, 1)
#  text(0, 0.5, "Clipping", cex = 2, srt = 30)
#  clip(-1, 0, 0, 1)
#  abline(h = 0.5, col = "red")
#})