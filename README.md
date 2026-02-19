# `httpgd` <img src="man/figures/httpgd_logo.svg" align="right" width="25%" alt="httpgd logo"/>

<!-- badges: start -->
[![R-CMD-check](https://github.com/nx10/httpgd/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/nx10/httpgd/actions/workflows/R-CMD-check.yaml)
[![CRAN](https://www.r-pkg.org/badges/version/httpgd)](https://CRAN.R-project.org/package=httpgd)
![downloads](https://cranlogs.r-pkg.org/badges/grand-total/httpgd)
[![Codecov test coverage](https://codecov.io/gh/nx10/httpgd/branch/master/graph/badge.svg)](https://app.codecov.io/gh/nx10/httpgd?branch=master)
<!-- badges: end -->

An HTTP/WebSocket graphics device for R. Serves plots asynchronously and includes an interactive web-based plot viewer. Built to embed live R graphics in IDEs and other applications.

Powered by [`unigd`](https://github.com/nx10/unigd).

## Features

* Fast, asynchronous plot rendering
* Interactive plot viewer with history and resizing
* Multiple concurrent clients
* Export to SVG, PNG, PDF, EPS, and more
* Stateless HTTP/WebSocket API

## Demo

![httpgd demo showing interactive plot viewer](https://user-images.githubusercontent.com/33600480/113182768-92eeda80-9253-11eb-9505-79de107024f7.gif)

## Installation

Install from CRAN:

```R
install.packages("httpgd")
```

Or install the development version from GitHub:

```R
remotes::install_github("nx10/httpgd")
```

See [system requirements](https://nx10.dev/httpgd/articles/a00_installation.html#system-requirements) for details.

### Documentation

- [Getting started](https://nx10.dev/httpgd/articles/a01_how-to-get-started.html)
- [Plotting in `unigd`](https://nx10.dev/unigd/articles/b00_guide.html)
- [Function reference](https://nx10.dev/httpgd/reference/index.html)
- [VS Code](https://nx10.dev/httpgd/articles/b01_vscode.html) / [RStudio](https://nx10.dev/httpgd/articles/b02_rstudio.html) / [Docker](https://nx10.dev/httpgd/articles/b03_docker.html)
- [HTTP/WebSocket API](https://nx10.dev/httpgd/articles/c01_httpgd-api.html)


## Contributing

`httpgd` is written in C++, R, and TypeScript. Contributions of any kind are welcome.

## Links

- [Using httpgd in VSCode: A web-based SVG graphics device](https://renkun.me/2020/06/16/using-httpgd-in-vscode-a-web-based-svg-graphics-device/)

## License

Licensed under GPL v2.0. Webserver based on [`CrowCpp/Crow`](https://github.com/CrowCpp/Crow). Vendored license copies are in [`./inst/licenses`](https://github.com/nx10/httpgd/tree/master/inst/licenses).
