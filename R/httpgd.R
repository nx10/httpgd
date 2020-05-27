

#' Initialize httpgd graphics device and start server.
#'
#' @param host Hostname
#' @param port Port
#' @param width Graphics device width (pixels)
#' @param height Graphics device height (pixels)
#' @param bg Background color
#' @param pointsize Graphics device point size
#' @param system_fonts Named list of font names to be aliased with
#'   fonts installed on your system. If unspecified, the R default
#'   families \code{sans}, \code{serif}, \code{mono} and \code{symbol}
#'   are aliased to the family returned by
#'   \code{\link[gdtools]{match_family}()}.
#' @param user_fonts Named list of fonts to be aliased with font files
#'   provided by the user rather than fonts properly installed on the
#'   system. The aliases can be fonts from the fontquiver package,
#'   strings containing a path to a font file, or a list containing
#'   \code{name} and \code{file} elements with \code{name} indicating
#'   the font alias in the SVG output and \code{file} the path to a
#'   font file.
#' @param recording Toggles plot history
#'
#' @export
httpgd <-
  function(host = "localhost",
           port = 8288,
           width = 720,
           height = 576,
           bg = "white",
           pointsize = 12,
           system_fonts = list(),
           user_fonts = list(),
           recording = TRUE) {
    aliases <- validate_aliases(system_fonts, user_fonts)
    httpgd_(host, port, bg, width, height, pointsize, aliases, recording)
    surl <- paste0("http://", host, ":", port)
    writeLines(paste0("httpgd live server running at:\n  ",surl,"/live"))
  }
