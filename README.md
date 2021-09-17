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
The device asynchronously serves graphics via HTTP and WebSockets.

## Features

* Fast plotting
* Plot resizing and history
* Interactive plot viewer (client)
* Platform independent
* Export to various image formats (SVG, PNG, PDF, EPS, ...)
* Multiple concurrent clients
* For developers:
  * HTML/JavaScript client (TypeScript module)
  * Stateless asynchronous HTTP/WebSocket API
  

## Demo

![demo](https://user-images.githubusercontent.com/33600480/113182768-92eeda80-9253-11eb-9505-79de107024f7.gif)

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
| <kbd>+</kbd> / <kbd>-</kbd> | Zoom in and out. |
| <kbd>0</kbd> | Reset zoom level. |
| <kbd>N</kbd> | Jump to the newest plot. |
| <kbd>del</kbd> / <kbd>D</kbd> | Delete plot. |
| <kbd>alt</kbd>+<kbd>D</kbd> | Clear all plots. |
| <kbd>S</kbd> | Download plot as SVG. |
| <kbd>P</kbd> | Download plot as PNG. |
| <kbd>C</kbd> | Copy plot to clipboard (as PNG). |
| <kbd>H</kbd> | Toggle plot history (sidebar). |

### API &amp; Documentation

The API documentation can be found [here](https://github.com/nx10/httpgd/blob/master/docs/api-documentation.md), and is also available as a package vignette.

Technical documentation for developers wanting to contribute to `httpgd` can be found [here](https://github.com/nx10/httpgd/blob/master/docs/tecdoc.md).

## Benchmark

There are currently no other network graphics devices for comparison, `httpgd` can be used in offline mode (with `hgd(webserver = FALSE)`) to compare it with conventional SVG graphics devices.

![](https://user-images.githubusercontent.com/33600480/113184973-232e1f00-9256-11eb-9595-327ec28ba360.png)

This benchmark compares `httpgd` 1.1.0 with `svglite` 2.0.0.

[See benchmark code](https://github.com/nx10/httpgd/blob/master/docs/benchmark.R)


## System requirements

Depends on `R` version &geq; 4.0 on windows, and R &geq; 3.2 on linux and macOS (a C++ compiler with basic C++17 support [is required](https://github.com/nx10/httpgd/issues/56)).

Note that there is a rare bug in R versions < 4.1, that leads to [some plots disappearing when ggplot2 plots are resized and deleted in a specific way](https://github.com/nx10/httpgd/issues/50).

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

If you feel lost, the [documentation](#api--documentation) might help.

## Links &amp; Articles

- [Using httpgd in VSCode: A web-based SVG graphics device](https://renkun.me/2020/06/16/using-httpgd-in-vscode-a-web-based-svg-graphics-device/)

## About &amp; License

Depends on `cpp11`, `later` and `systemfonts`.

Webserver based on [`Boost/Beast`](<https://github.com/boostorg/beast>) included in the `BH` package.
    
Much of the font handling and SVG rendering code is modified code from the excellent [`svglite`](<https://github.com/r-lib/svglite>) package.

This project is licensed GPL v2.0.

It includes parts of [`svglite`](<https://github.com/r-lib/svglite>) (GPL &geq; 2), [`Belle`](https://github.com/octobanana/belle) (MIT) and [`fmt`](https://github.com/fmtlib/fmt) (MIT). The HTML client includes [Material Design icons by Google](https://github.com/google/material-design-icons) which are licensed under the [Apache License Version 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt).

Full copies of the license agreements used by these components are included in [`./inst/licenses`](https://github.com/nx10/httpgd/tree/master/inst/licenses).