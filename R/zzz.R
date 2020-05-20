#' httpgd: Http server graphics device
#' 
#' Http server graphics device
#'
#' @docType package
#' @name httpgd-package
#' @import later
#' @useDynLib httpgd, .registration=TRUE
NULL


#' @importFrom grDevices dev.list dev.off
.onUnload <- function (libpath) {
  
  # search for all active servers and close them
  ds <- dev.list()
  lapply(ds[names(ds) == "httpgd"], dev.off)
  
  library.dynam.unload("httpgd", libpath)
}
