# `httpgd` <img src="docs/httpgd_logo.svg" align="right" height = 250/>

<!-- badges: start -->
[![R-CMD-check](https://github.com/nx10/httpgd/workflows/R-CMD-check/badge.svg)](https://github.com/nx10/httpgd/actions)
[![CRAN](https://www.r-pkg.org/badges/version/httpgd)](https://CRAN.R-project.org/package=httpgd)
![downloads](https://cranlogs.r-pkg.org/badges/grand-total/httpgd)
<!-- badges: end -->

A graphics device for R that is accessible via network protocols.
This package was created to make it easier to embed live R graphics in 
integrated development environments and other applications.
The included HTML/JavaScript client (plot viewer) aims to provide a better overall user experience when dealing with R graphics.
The device asynchronously serves SVG graphics via HTTP and WebSockets.

## Features

* Fast high quality SVG plots
* Stateless asynchronous HTTP/WebSocket API
* Plot resizing
* Plot history
* HTML/JavaScript client (TypeScript module)
* Multiple concurrent clients

## Demo

![demo](https://user-images.githubusercontent.com/33600480/83944385-6587fa80-a803-11ea-8f4a-7808d144309d.gif)

## Installation

Install `httpgd` from CRAN:

```R
install.packages("httpgd")
```

Or get the latest development version from GitHub:

```R
devtools::install_github("nx10/httpgd")
```

See [system requirements](#System-requirements) for troubleshooting.

## Usage

Initialize graphics device and start server with:

```R
hgd()
```

Copy the displayed link in the browser or call

```R
hgd_browse()
```

to open a browser window automatically.

Plot anything.

```R
x = seq(0, 3 * pi, by = 0.1)
plot(x, sin(x), type = "l")
```

Every plotting library will work.

```R
library(ggplot2)
ggplot(mpg, aes(displ, hwy, colour = class)) +
  geom_point()
```

Stop the server with:

```R
dev.off()
```

### Keyboard shortcuts

| Keys | Result |
|:----:|--------|
| <kbd>&#8592;</kbd> <kbd>&#8594;</kbd> <kbd>&#8593;</kbd> <kbd>&#8595;</kbd> | Navigate plot history. |
| <kbd>N</kbd> | Jump to the newest plot. |
| <kbd>del</kbd> / <kbd>D</kbd> | Delete plot. |
| <kbd>+</kbd> / <kbd>-</kbd> | Zoom in and out. |
| <kbd>0</kbd> | Reset zoom level. |
| <kbd>S</kbd> | Download plot as SVG. |
| <kbd>P</kbd> | Download plot as PNG. |
| <kbd>C</kbd> | Copy plot to clipboard (as PNG). |
| <kbd>T</kbd> | Clear all plots. |

### API &amp; Documentation

`httpgd` can be accessed both from R and from HTTP:

* [R API](https://github.com/nx10/httpgd/blob/master/docs/RApi.md)
* [Web API](https://github.com/nx10/httpgd/blob/master/docs/WebApi.md)

Technical documentation for developers wanting to contribute can be found [here](https://github.com/nx10/httpgd/blob/master/docs/tecdoc.md).

## Benchmark

There are currently no other network graphics devices for comparison, `httpgd` can be used in offline mode (with `hgd(webserver = FALSE)`) to compare it with conventional SVG graphics devices.

The [benchmark from svglite](https://github.com/r-lib/svglite/blob/master/README.md) has the following results:

<details>

<summary>Code</summary>

```R
library(svglite)
library(httpgd)
set.seed(1234)
x <- runif(1e3)
y <- runif(1e3)
tmp1 <- tempfile()
tmp2 <- tempfile()
tmp3 <- tempfile()
svglite_test <- function() {
  svglite(tmp1)
  plot(x, y)
  dev.off()
}
svg_test <- function() {
  svg(tmp2, onefile = TRUE)
  plot(x, y)
  dev.off()
}
httpgd_test <- function() {
  hgd(webserver = FALSE)
  plot(x, y)
  hgd_svg(file = tmp3)
  dev.off()
}
ben <-
  bench::mark(httpgd_test(), svglite_test(), svg_test(), iterations = 250)
```

[See full code](https://github.com/nx10/httpgd/blob/master/docs/benchmark.R)

</details>

|expression     |    min| median|  itr/sec| mem_alloc|    gc/sec| n_itr| n_gc| total_time|
|:--------------|------:|------:|--------:|---------:|---------:|-----:|----:|----------:|
|httpgd_test()  | 10.2ms| 10.8ms| 91.43165|     361KB| 0.7373520|   248|    2|      2.71s|
|svglite_test() | 20.3ms| 21.4ms| 46.29561|     593KB| 0.5622948|   247|    3|      5.33s|
|svg_test()     | 27.2ms| 28.3ms| 35.15964|     126KB| 0.1412034|   249|    1|      7.08s|

*Package versions: `httpgd` (1.0.0 dev), `svglite` (1.2.3.2), `grDevices` (4.0.3)*

<img src="https://raw.githubusercontent.com/nx10/httpgd/master/docs/bench_speed1.png" width="640"/> <img src="https://raw.githubusercontent.com/nx10/httpgd/master/docs/bench_size1.png" width="640"/>


## System requirements

Depends on `R` version &geq; 4.0 on windows, and R &geq; 3.2 on linux and macOS (a C++ compiler with basic C++17 support [is required](https://github.com/nx10/httpgd/issues/56)).

Note that there is a rare bug in R versions < 4.1, that leads to [some plots dissappearing when ggplot2 plots are resized and deleted in a specific way](https://github.com/nx10/httpgd/issues/50).

`libpng` and X11 are required on unix like systems (e.g. Linux, macOS).

### macOS

If `libpng` is missing install it via:

```sh
brew install libpng
```

If `X11` is missing the error message will include the text:

```sh
unable to load shared object [...] systemfonts/libs/systemfonts.so [...]
```

Install [`XQuartz`](https://www.xquartz.org/).
(see: <https://github.com/r-lib/systemfonts/issues/17>)

## Help welcome!

The various components of `httpgd` are written in C++, R and TypeScript. We welcome contributions of any kind.

Other areas in need of improvement are: Testing, documentation, net security and continuous integration.

If you feel lost, the [technical documentation](https://github.com/nx10/httpgd/blob/master/docs/tecdoc.md) might help.

## Links &amp; Articles

- [Using httpgd in VSCode: A web-based SVG graphics device](https://renkun.me/2020/06/16/using-httpgd-in-vscode-a-web-based-svg-graphics-device/)

## About &amp; License

Depends on `cpp11`, `later` and `systemfonts`.

Webserver based on [`Boost/Beast`](<https://github.com/boostorg/beast>) included in the `BH` package.
    
Much of the font handling and SVG rendering code is modified code from the excellent [`svglite`](<https://github.com/r-lib/svglite>) package.

This project is licensed GPL v2.0.

It includes parts of [`svglite`](<https://github.com/r-lib/svglite>) (GPL &geq; 2), [`Belle`](https://github.com/octobanana/belle) (MIT) and [`fmt`](https://github.com/fmtlib/fmt) (MIT). The HTML client includes [Material Design icons by Google](https://github.com/google/material-design-icons) which are licensed under the [Apache License Version 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt).

Full copies of the license agreements used by these components are included in [`./inst/licenses`](https://github.com/nx10/httpgd/tree/master/inst/licenses).