# On posix all requests mus be done from an extra session
# otherwise the main session will deadlock.
fetch_get <- function(...) {
    if (.Platform$OS.type == "windows") {
        return(
            httr::GET(...)
        )
    } else {
        return(
            (function() {
                future::plan("multisession")
                v <- future::value(future::future({
                    return(httr::GET(...))
                }))
                future::plan("sequential")
                v
            })()
        )
    }
  }