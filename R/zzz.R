

.onUnload <- function (libpath) {
  
  # todo: search for all active servers and close them
  if (names(dev.cur()) == "httpgd") {
    dev.off() 
  }
  
  library.dynam.unload("httpgd", libpath)
}
