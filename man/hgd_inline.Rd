% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/httpgd.R
\name{hgd_inline}
\alias{hgd_inline}
\title{Inline SVG rendering.}
\usage{
hgd_inline(code, page = 0, page_width = -1, page_height = -1, file = NA, ...)
}
\arguments{
\item{code}{Plotting code. See examples for more information.}

\item{page}{Plot page to render. If this is set to \code{0}, the last page will
be selected. Can be set to a numeric plot index or plot ID
(see \code{\link[=hgd_id]{hgd_id()}}).}

\item{page_width}{Width of the plot. If this is set to \code{-1}, the last width
will be selected.}

\item{page_height}{Height of the plot. If this is set to \code{-1}, the last
height will be selected.}

\item{file}{Filepath to save SVG. (No file will be created if this is \code{NA})}

\item{...}{Additional parameters passed to \code{hgd(webserver=FALSE, ...)}}
}
\value{
Rendered SVG string.
}
\description{
Convenience function for quick inline SVG rendering.
This is similar to \code{\link[=hgd_svg]{hgd_svg()}} but the plotting code is specified inline
and an offline httpgd graphics device is managed (created and closed)
automatically. Starting a device with \code{\link[=hgd]{hgd()}} is therefore not necessary.
}
\examples{
hgd_inline({
  hist(rnorm(100))
})

s <- hgd_inline({
  plot.new()
  lines(c(0.5, 1, 0.5), c(0.5, 1, 1))
})
cat(s)
}
