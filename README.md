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

Built using [Boost Beast](https://github.com/boostorg/beast) and [Belle](https://github.com/octobanana/belle).

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
| <kbd>del</kbd> | Remove current plot. |
| <kbd>+</kbd> / <kbd>-</kbd> | Zoom in and out. |
| <kbd>0</kbd> | Reset zoom level. |
| <kbd>ctrl</kbd> + <kbd>S</kbd> | Download plot as SVG. |
| <kbd>ctrl</kbd> + <kbd>R</kbd> | Download plot as PNG. |

### API

![structure](docs/httpgd_structure.svg)

httpgd can be accessed both from R and from HTTP:

* [R API](docs/RApi.md)
* [Web API](docs/WebApi.md)

## Planned features

* Generate optimized (as small as possible) SVGs.
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
