

#' Initialize httpgd graphics device and start server.
#'
#' @param host Server hostname. Set to "0.0.0.0" to enable remote access.
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
#'   \code{\link[systemfonts]{font_info}()}.
#' @param user_fonts Named list of fonts to be aliased with font files
#'   provided by the user rather than fonts properly installed on the
#'   system. The aliases can be fonts from the fontquiver package,
#'   strings containing a path to a font file, or a list containing
#'   \code{name} and \code{file} elements with \code{name} indicating
#'   the font alias in the SVG output and \code{file} the path to a
#'   font file.
#' @param cors Toggles Cross-Origin Resource Sharing (CORS) header.
#'   When set to TRUE, CORS header will be set to "*".
#' @param token (Optional) security token. When set, all requests
#'   need to include a token to be allowed. (Either in a request header 
#'   (X-HTTPGD-TOKEN) field or as a query parameter.)
#'   This parameter can be set to TRUE to generate a random 8 character
#'   alphanumeric token. A random token of the specified length is generated
#'   when it is set to a number. FALSE deactivates the token.
#' @param silent When set to FALSE no information will be printed to console.
#' @param websockets Use websockets.
#' @param webserver Can be set to FALSE for offline mode.
#' @param fix_text_width Should the width of strings be fixed so that it doesn't
#'   change between svg renderers depending on their font rendering? Defaults to
#'   `TRUE`. If `TRUE` each string will have the `textLength` CSS property set
#'   to the width calculated by systemfonts and
#'   `lengthAdjust='spacingAndGlyphs'`. Setting this to `FALSE` can be
#'   beneficial for heavy post-processing that may change content or style of
#'   strings, but may lead to inconsistencies between strings and graphic
#'   elements that depend on the dimensions of the string (e.g. label borders
#'   and background).
#'
#' @importFrom systemfonts match_font
#' @export
#' 
#' @examples
#' \dontrun{
#' 
#' hgd() # Initialize graphics device and start server
#' # hgd_browse() # Copy the displayed link in the browser or call
#' 
#' # Plot something
#' x = seq(0, 3 * pi, by = 0.1)
#' plot(x, sin(x), type = "l")
#' 
#' dev.off() # alternatively: hgd_close()
#' }
hgd <-
  function(host = "127.0.0.1",
           port = 0,
           width = 720,
           height = 576,
           bg = "white",
           pointsize = 12,
           system_fonts = list(),
           user_fonts = list(),
           cors = FALSE,
           token = TRUE,
           silent = FALSE,
           websockets = TRUE,
           webserver = TRUE, 
           fix_text_width = TRUE) {
    
    tok <- ""
    if (is.character(token)) {
      tok <- token
    } else if (is.numeric(token)) {
      tok <- httpgd_random_token_(token)
    } else if (is.logical(token) && token) {
      tok <- httpgd_random_token_(8)
    }
    
    aliases <- validate_aliases(system_fonts, user_fonts)
    if (httpgd_(host, port, bg, width, height, pointsize, aliases, cors, tok, webserver, silent, fix_text_width)) {
      if (!silent && webserver) {
        cat("httpgd server running at:\n  ",
          hyperrefstyle(hgd_url(websockets = websockets)),
          "\n", sep = "")
      }
    } else {
      hgd_close()
      stop("Failed to start server. (Port might be in use.)")
    }
  }

#' Adds ANSI escape codes to string if terminal supports it.
#'
#' @param str The string to style
hyperrefstyle <- function(str) {
  if (grepl("^screen|^xterm|^vt100|color|ansi|cygwin|linux", 
        Sys.getenv("TERM"), ignore.case = TRUE, perl = TRUE)) {
          return(paste0("\033[4m\033[34m", str, "\033[39m\033[24m"))
        }
  return(str)
}

#' Returns status information of a httpgd graphics device.
#'
#' @param which Which device (id)
#'
#' @return List of status variables
#'
#' @importFrom grDevices dev.cur
#' @export
#' 
#' @examples
#' \dontrun{
#' 
#' hgd()
#' 
#' hgd_state()
#' 
#' plot(1,1)
#' 
#' hgd_state() 
#' 
#' dev.off() 
#' }
hgd_state <- function(which = dev.cur()) {
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
#' @param which Which device (id).
#' @param file Filepath to save SVG. (No file will be created if this is NA)
#'
#' @return Rendered SVG string.
#'
#' @importFrom grDevices dev.cur
#' @export
#' 
#' @examples
#' \dontrun{
#' 
#' hgd()
#' plot(1,1)
#' s <- hgd_svg(width=600, height=400)
#' hist(rnorm(100))
#' hgd_svg(file="my_plot.svg", width=600, height=400) 
#' dev.off()
#' }
hgd_svg <- function(page = 0, width = -1, height = -1, which = dev.cur(), file = NA) {
  if (names(which) != "httpgd") {
    stop("Device is not of type httpgd")
  }
  else {
    svg <- httpgd_svg_(which, page - 1, width, height)
    if (!is.na(file)) {
      cat(svg, file=file)
    }
    return(svg)
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
#' 
#' @examples
#' \dontrun{
#' 
#' hgd()
#' plot(1,1) # page 1
#' hist(rnorm(100)) # page 2
#' hgd_remove(page=1) # remove page 1
#' dev.off()
#' }
hgd_remove <- function(page = 0, which = dev.cur()) {
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
#' 
#' @examples
#' \dontrun{
#' 
#' hgd()
#' plot(1,1)
#' hist(rnorm(100))
#' hgd_clear()
#' hist(rnorm(100))
#' dev.off()
#' }
hgd_clear <- function(which = dev.cur()) {
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
#' @param websockets Use websockets
#'
#' @return URL
#'
#' @importFrom grDevices dev.cur
#' @export
#' 
#' @examples
#' \dontrun{
#' 
#' hgd()
#' my_url <- hgd_url()
#' #dev.off()
#' }
hgd_url <- function(endpoint = "live", which = dev.cur(), websockets = TRUE) {
  l <- hgd_state(which)
  paste0("http://",
         sub('0.0.0.0', Sys.info()[['nodename']], l$host, fixed = TRUE),
         ":",
         l$port,
         "/",
         endpoint,
         ifelse(nchar(l$token) == 0, "", paste0("?token=", l$token)),
         ifelse(nchar(l$token) == 0, ifelse(websockets, "", "?ws=0"), ifelse(websockets, "", "&ws=0")))
}

#' Opens the URL by which a httpgd device is accessible in a browser.
#'
#' @param endpoint API endpoint
#' @param which Which device (id)
#'
#' @return URL
#'
#' @importFrom grDevices dev.cur
#' @importFrom utils browseURL
#' @export
#' 
#' @examples
#' \dontrun{
#' 
#' hgd()
#' hgd_browse() # open browser
#' hist(rnorm(100))
#' #dev.off()
#' }
hgd_browse <- function(endpoint = "live", which = dev.cur()) {
  browseURL(hgd_url(endpoint, which))
}

#' Closes a graphics device if it is a httpgd server.
#' This achieves the same effect as dev.off().
#'
#' @param which Which device (id)
#' @param all If set to true, all running httpgd devices will be closed
#' 
#' @importFrom grDevices dev.cur dev.list dev.off
#' @export
#' 
#' @examples
#' \dontrun{
#' 
#' hgd()
#' hgd_browse() # open browser
#' hist(rnorm(100))
#' # ...
#' hgd_close() # Equvalent to dev.off()
#' 
#' hgd()
#' hgd()
#' hgd()
#' hgd_close(all=TRUE)
#' }
hgd_close <- function(which = dev.cur(), all = FALSE) {
  if (all) {
    ds <- dev.list()
    invisible(lapply(ds[names(ds) == "httpgd"], dev.off))
  } else {
    if (names(which(dev.list() == which)) == "httpgd") {
      dev.off(which)
    }
  }
}

#' Generate a random alphanumeric token string. 
#' This is mainly used internally by httpgd.
#'
#' @param len Token length
#'
#' @return Random token string
#' @export
#' 
#' @examples
#' \dontrun{
#' 
#' hgd_generate_token(6)
#' }
hgd_generate_token <- function(len) {
  httpgd_random_token_(len)
}


#' Returns a plot rendered as a SVG string.
#' This is similar to hgd_svg(...) but the plotting code is specified inline.
#' This function manages (creates and destroys) the httpgd graphics device by itself.
#'
#' @param code Plotting code.
#' @param page Plot page to render. If this is set to 0, the last page will be selected. 
#' @param page_width Width of the plot. If this is set to -1, the last width will be selected. 
#' @param page_height Height of the plot. If this is set to -1, the last height will be selected.
#' @param file Filepath to save SVG. (No file will be created if this is NA)
#' @param ... Additional parameters passed to hgd(webserver=FALSE, ...)
#'
#' @return Rendered SVG string.
#' @export
#'
#' @examples
#' \dontrun{
#' 
#' hgd_inline({
#'   hist(rnorm(100))
#' })
#' 
#' s <- hgd_inline({
#'   plot.new()
#'   lines(c(0.5, 1, 0.5), c(0.5, 1, 1))
#' })
#' cat(s)
#' }
hgd_inline <- function(code, page = 0, page_width = -1, page_height = -1, file = NA, ...) {
  hgd(webserver=FALSE, ...)
  tryCatch(code,
    finally = {
      s <- hgd_svg(page=page, width=page_width, height=page_height)
      dev.off()
    }
  )
  s
}