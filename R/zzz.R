#' httpgd: Http server graphics device
#' 
#' Http server graphics device
#'
#' @docType package
#' @name httpgd-package
#' @useDynLib httpgd, .registration=TRUE
NULL


#' @importFrom grDevices dev.cur dev.off
.onUnload <- function (libpath) {
  
  # todo: search for all active servers and close them
  if (names(dev.cur()) == "httpgd") {
    dev.off() 
  }
  
  library.dynam.unload("httpgd", libpath)
}
