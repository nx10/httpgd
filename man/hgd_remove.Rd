% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/httpgd.R
\name{hgd_remove}
\alias{hgd_remove}
\title{Remove a httpgd plot page.}
\usage{
hgd_remove(page = 0, which = dev.cur())
}
\arguments{
\item{page}{Plot page to remove. If this is set to \code{0}, the last page will
be selected. Can be set to a numeric plot index or plot ID
(see \code{\link[=hgd_id]{hgd_id()}}).}

\item{which}{Which device (ID).}
}
\value{
Whether the page existed (and thereby was successfully removed).
}
\description{
This function will only work after starting a device with \code{\link[=hgd]{hgd()}}.
}
\examples{
\dontrun{

hgd()
plot(1, 1) # page 1
hist(rnorm(100)) # page 2
hgd_remove(page = 1) # remove page 1

dev.off()
}
}
