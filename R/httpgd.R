

#' Initialize httpgd graphics device and start server.
#'
#' @param host Server hostname.
#' @param port Server port. If this is set to 0, an open port 
#'   will be assigned.
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
#' @param token (Optional) security token. When set, all requests
#'   need to include a token to be allowed. (Either in a request header 
#'   (X-HTTPGD-TOKEN) field or as a query parameter.)
#'   This parameter can be set to TRUE to generate a random 8 character
#'   alphanumeric token. A random token of the specified length is generated
#'   when it is set to a number. FALSE deactivates the token.
#' @param silent When set to FALSE no information will be printed to the 
#'   console after startup.
#'
#' @export
httpgd <-
  function(host = "127.0.0.1",
           port = 0,
           width = 720,
           height = 576,
           bg = "white",
           pointsize = 12,
           system_fonts = list(),
           user_fonts = list(),
           recording = TRUE,
           cors = FALSE,
           token = TRUE,
           silent = FALSE) {
    
    tok <- ""
    if (is.character(token)) {
      tok <- token
    } else if (is.numeric(token)) {
      tok <- httpgd_random_token_(token)
    } else if (is.logical(token) && token) {
      tok <- httpgd_random_token_(8)
    }
    
    aliases <- validate_aliases(system_fonts, user_fonts)
    httpgd_(host, port, bg, width, height, pointsize, aliases, recording, cors, tok)
    if (!silent) {
      writeLines(paste0("httpgd live server running at:\n  ", httpgdURL()))
    }
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

#' Returns a plot rendered as a SVG string.
#'
#' @param page Plot page to render. If this is set to 0, the last page will be selected. 
#' @param width Width of the plot. If this is set to -1, the last width will be selected. 
#' @param height Height of the plot. If this is set to -1, the last height will be selected. 
#' @param which Which device (id)
#'
#' @return Rendered SVG string.
#'
#' @importFrom grDevices dev.cur
#' @export
httpgdSVG <- function(page = 0, width = -1, height = -1, which = dev.cur()) {
  if (names(which) != "httpgd") {
    stop("Device is not of type httpgd")
  }
  else {
    return(httpgd_svg_(which, page - 1, width, height))
  }
}

#' Removes a plot page.
#'
#' @param page Plot page to remove. If this is set to 0, the last page will be selected. 
#' @param which Which device (id)
#'
#' @return Whether the page existed (and thereby was successfully removed).
#'
#' @importFrom grDevices dev.cur
#' @export
httpgdRemove <- function(page = 0, which = dev.cur()) {
  if (names(which) != "httpgd") {
    stop("Device is not of type httpgd")
  }
  else {
    return(httpgd_remove_(which, page - 1))
  }
}

#' Clears all plot pages.
#'
#' @param which Which device (id)
#'
#' @return Whether there were any pages to remove.
#'
#' @importFrom grDevices dev.cur
#' @export
httpgdClear <- function(which = dev.cur()) {
  if (names(which) != "httpgd") {
    stop("Device is not of type httpgd")
  }
  else {
    return(httpgd_clear_(which))
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

#' Opens the url by which a httpgd device is accessible in a browser.
#'
#' @param endpoint API endpoint
#' @param which Which device (id)
#'
#' @return URL
#'
#' @importFrom grDevices dev.cur
#' @importFrom utils browseURL
#' @export
httpgdBrowse <- function(endpoint = "live", which = dev.cur()) {
  browseURL(httpgdURL(endpoint, which))
}

#' Closes a graphics device if it is a httpgd server.
#' This achieves the same effect as dev.off().
#'
#' @param which Which device (id)
#' 
#' @importFrom grDevices dev.cur dev.list dev.off
#' @export
httpgdCloseServer <- function(which = dev.cur()) {
  if (names(which(dev.list() == which)) == "httpgd") {
    dev.off(which)
  }
}

#' Search for all active servers and close them.
#'
#' @export
httpgdCloseAllServers <- function() {
  ds <- dev.list()
  invisible(lapply(ds[names(ds) == "httpgd"], dev.off))
}
