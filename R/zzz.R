#' httpgd: Http server graphics device
#' 
#' Asynchronous http server graphics device.
#'
#' @docType package
#' @name httpgd-package
#' @import later
#' @useDynLib httpgd, .registration=TRUE
NULL

.cleanOldPlotHistories <- function() {
  myls <- ls(all.names = T, envir = globalenv())
  oldph <- myls[startsWith(myls, ".httpgdPlots_")]
  if (length(oldph) > 0) {
    warning("Cleaned up previous httpgd plot histories.")
    rm(list = myls[startsWith(myls, ".httpgdPlots_")], envir = globalenv())
  }
}

.onLoad <- function(libname, pkgname){
  .cleanOldPlotHistories()
}

#' @importFrom grDevices dev.list dev.off
.onUnload <- function (libpath) {
  hgd_close(all = TRUE)
  
  library.dynam.unload("httpgd", libpath)
}
