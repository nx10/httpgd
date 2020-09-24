# httpgd <img src="docs/httpgd_logo.svg" align="right" height = 250/>

[![R build status](https://github.com/nx10/httpgd/workflows/build/badge.svg)](https://github.com/nx10/httpgd/actions)

Asynchronous http server graphics device for R.

## Features

* Fast high quality plots
* Stateless HTTP/Websocket SVG API
* Plot resizing
* Plot history
* HTML/JavaScript client (TypeScript module)
* Multiple concurrent clients

## Demo

![demo](https://user-images.githubusercontent.com/33600480/83944385-6587fa80-a803-11ea-8f4a-7808d144309d.gif)

## Installation

```R
devtools::install_github("nx10/httpgd")
```

Depends on `Rcpp`, `later` and `systemfonts`.

SVG rendering (especially font rendering) based on `svglite` (<https://github.com/r-lib/svglite>).

Built using [Boost Beast](https://github.com/boostorg/beast), [Belle](https://github.com/octobanana/belle) and [fmt](https://github.com/fmtlib/fmt).

See [system requirements](#System-requirements) for troubleshooting.

## Usage

Initialize graphics device and start server with:

```R
httpgd()
```

Copy the displayed link in the browser or call

```R
httpgdBrowse()
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
| <kbd>R</kbd> | Download plot as PNG. |
| <kbd>C</kbd> | Clear all plots. |

### API

![structure](docs/httpgd_structure.svg)

httpgd can be accessed both from R and from HTTP:

* [R API](docs/RApi.md)
* [Web API](docs/WebApi.md)

## Benchmark

The following benchmark (expanded from [svglite](https://github.com/r-lib/svglite/blob/master/README.md))

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
  httpgd(webserver = FALSE)
  plot(x, y)
  httpgdSVG(file = tmp3)
  dev.off()
}
ben <-
  bench::mark(httpgd_test(), svglite_test(), svg_test(), iterations = 250)
```

has the following results:

|expression     |    min| median|  itr/sec| mem_alloc|    gc/sec| n_itr| n_gc| total_time|
|:--------------|------:|------:|--------:|---------:|---------:|-----:|----:|----------:|
|httpgd_test()  | 10.3ms|   11ms| 90.03988|     363KB| 0.3616059|   249|    1|      2.77s|
|svglite_test() | 20.7ms| 21.3ms| 46.02538|     593KB| 0.5590127|   247|    3|      5.37s|
|svg_test()     | 27.2ms| 28.3ms| 34.52923|     126KB| 0.0000000|   250|    0|      7.24s|

*Package versions: svglite (1.2.3.2), grDevices (4.0.2), httpgd (0.4.0)*

<img src="https://raw.githubusercontent.com/nx10/httpgd/master/docs/bench_speed1.png" width="640"/> <img src="https://raw.githubusercontent.com/nx10/httpgd/master/docs/bench_size1.png" width="640"/>

## Planned features

* TLS encryption (HTTPS/WSS)

## System requirements

`libpng` and X11 are required on unix like systems (e.g. Linux, macOS).

### macOS

If `libpng` is missing install it via:

```sh
brew install libpng
```

If X11 is missing the error message will include the text:

```sh
unable to load shared object [...] systemfonts/libs/systemfonts.so [...]
```

Install [XQuartz](https://www.xquartz.org/).
(see: <https://github.com/r-lib/systemfonts/issues/17>)

## Help welcome

Any advice and suggestions are welcome. Especially with C++ optimization and security.

## License

This project is licensed GPL v2.0.

The HTML client includes [Material Design icons by Google](https://github.com/google/material-design-icons) which are licensed under the [Apache License Version 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt).
