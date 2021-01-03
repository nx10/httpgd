#' httpgd: Http server graphics device
#' 
#' Asynchronous http server graphics device.
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
