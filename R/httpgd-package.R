#' httpgd: HTTP server graphics device
#'
#' Asynchronous HTTP server graphics device.
#'
#' @name httpgd-package
#' @useDynLib httpgd, .registration=TRUE
"_PACKAGE"

.onLoad <- function(libname, pkgname) {
}

#' @importFrom grDevices dev.list dev.off
.onUnload <- function(libpath) {
  hgd_close(all = TRUE)
  library.dynam.unload("httpgd", libpath)
}
