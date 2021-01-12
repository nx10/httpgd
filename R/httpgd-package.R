#' httpgd: HTTP server graphics device
#' 
#' Asynchronous HTTP server graphics device.
#'
#' @docType package
#' @name httpgd-package
#' @import later
#' @useDynLib httpgd, .registration=TRUE
NULL

#' @importFrom grDevices dev.list dev.off
.onUnload <- function (libpath) {
  hgd_close(all = TRUE)
  library.dynam.unload("httpgd", libpath)
}
