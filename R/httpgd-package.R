#' httpgd: HTTP server graphics device
#'
#' Asynchronous HTTP server graphics device.
#'
#' @name httpgd-package
#' @useDynLib httpgd, .registration=TRUE
"_PACKAGE"

.onLoad <- function(libname, pkgname) {
  #httpgd_ipc_open_()
}

#' @importFrom grDevices dev.list dev.off
.onUnload <- function (libpath) {
  hgd_close(all = TRUE)
  #httpgd_ipc_close_()
  library.dynam.unload("httpgd", libpath)
}
