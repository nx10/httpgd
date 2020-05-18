

httpgd <-
  function(host = "localhost",
           port = 1234,
           width = 720,
           height = 576,
           bg = "white",
           pointsize = 12,
           system_fonts = list(),
           user_fonts = list()) {
    aliases <- validate_aliases(system_fonts, user_fonts)
    httpgd_(host, port, bg, width, height, pointsize, aliases)
    surl <- paste0("http://", host, ":", port)
    writeLines(paste0("httpgd live server running at:\n  ",surl,"/live"))
  }
