VERSION <- commandArgs(TRUE)
if(!file.exists(sprintf("../windows/cairo-%s/include/cairo/cairo.h", VERSION))){
  if(getRversion() < "3.3.0") setInternet2()
  download.file(sprintf("https://github.com/rwinlib/cairo/archive/v%s.zip", VERSION), "libcairo.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("libcairo.zip", exdir = "../windows")
  unlink("libcairo.zip")
}