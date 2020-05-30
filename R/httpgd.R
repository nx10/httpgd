

#' Initialize httpgd graphics device and start server.
#'
#' @param host Hostname.
#' @param port Port.
#' @param width Graphics device width (pixels).
#' @param height Graphics device height (pixels).
#' @param bg Background color.
#' @param pointsize Graphics device point size.
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
#' @param recording Should a plot history be recorded.
#' @param cors Toggles Cross-Origin Resource Sharing (CORS) header.
#'   When set to TRUE, CORS header will be set to "*".
#' @param token (Optional) security token string. When set all requests
#'   need to include this token to be allowed. (Either in a request header 
#'   (X-HTTPGD-TOKEN) field or as a query parameter.)
#'
#' @export
httpgd <-
  function(host = "127.0.0.1",
           port = 8288,
           width = 720,
           height = 576,
           bg = "white",
           pointsize = 12,
           system_fonts = list(),
           user_fonts = list(),
           recording = TRUE,
           cors = FALSE,
           token = "") {
    aliases <- validate_aliases(system_fonts, user_fonts)
    httpgd_(host, port, bg, width, height, pointsize, aliases, recording, cors, token)
    surl <- paste0("http://", host, ":", port)
    writeLines(paste0("httpgd live server running at:\n  ",surl,"/live",ifelse(nchar(token)==0,"",paste0("?token=",token))))
  }


#' Returns status information of a httpgd graphics device.
#'
#' @param which Which device (id)
#'
#' @return List of status variables
#'
#' @importFrom grDevices dev.cur
#' @export
httpgdState <- function(which = dev.cur()) {
  if (names(which) != "httpgd") {
    stop("Device is not of type httpgd")
  }
  else {
    return(httpgd_state_(which))
  }
}


#' Returns the url by which a httpgd device is accessible.
#'
#' @param endpoint API endpoint
#' @param which Which device (id)
#'
#' @return URL
#'
#' @importFrom grDevices dev.cur
#' @export
httpgdURL <- function(endpoint = "live", which = dev.cur()) {
  l <- httpgdState(which)
  paste0("http://",
         l$host,
         ":",
         l$port,
         "/",
         endpoint,
         ifelse(nchar(l$token) == 0, "", paste0("?token=", l$token)))
}
