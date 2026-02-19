# `httpgd` <img src="man/figures/httpgd_logo.svg" align="right" width="25%"/>

<!-- badges: start -->
[![R-CMD-check](https://github.com/nx10/httpgd/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/nx10/httpgd/actions/workflows/R-CMD-check.yaml)
[![CRAN](https://www.r-pkg.org/badges/version/httpgd)](https://CRAN.R-project.org/package=httpgd)
![downloads](https://cranlogs.r-pkg.org/badges/grand-total/httpgd)
[![Codecov test coverage](https://codecov.io/gh/nx10/httpgd/branch/master/graph/badge.svg)](https://app.codecov.io/gh/nx10/httpgd?branch=master)
<!-- badges: end -->

A graphics device for R that is accessible via network protocols.
This package was created to make it easier to embed live R graphics in 
integrated development environments and other applications.
The included HTML/JavaScript client (plot viewer) aims to provide a better overall user experience when dealing with R graphics.
The device asynchronously serves graphics via HTTP and WebSockets.

## Features

* Fast plotting
* Interactive plot viewer (client)
* Supports multiple clients concurrently
* Plot resizing and history
* Export various image formats (SVG, PNG, PDF, EPS, ...)
* Powered by [`unigd`](https://github.com/nx10/unigd)
* For developers:
  * Stateless asynchronous HTTP/WebSocket API
  * HTML/JavaScript client (TypeScript module)

## Demo

![](https://user-images.githubusercontent.com/33600480/113182768-92eeda80-9253-11eb-9505-79de107024f7.gif)

## Installation

Install `httpgd` from CRAN:

```R
install.packages("httpgd")
```

Or get the latest development version from GitHub:

```R
remotes::install_github("nx10/httpgd")
```

See [system requirements](https://nx10.dev/httpgd/articles/a00_installation.html#system-requirements) for troubleshooting.


### Documentation

- For users:
  - [How to get started](https://nx10.dev/httpgd/articles/a01_how-to-get-started.html)
  - [Plotting in `unigd`](https://nx10.dev/unigd/articles/b00_guide.html)
  - [Function reference](https://nx10.dev/httpgd/reference/index.html)
  <!-- - [Benchmarks](https://nx10.dev/httpgd/articles/c02_benchmarks.html) -->
  - IDEs &amp; evironments:
    - [VS Code](https://nx10.dev/httpgd/articles/b01_vscode.html)
    - [RStudio](https://nx10.dev/httpgd/articles/b02_rstudio.html)
    - [Docker](https://nx10.dev/httpgd/articles/b03_docker.html)
- For package developers:
  - [httpgd API](https://nx10.dev/httpgd/articles/c01_httpgd-api.html)
  <!-- - [Technical documentation](https://nx10.dev/httpgd/articles/c03_technical-docs.html) -->


## Contributions welcome!

The various components of `httpgd` are written in C++, R and TypeScript. We welcome contributions of any kind.

Other areas in need of improvement are testing and documentation.

## Links &amp; Articles

- [Using httpgd in VSCode: A web-based SVG graphics device](https://renkun.me/2020/06/16/using-httpgd-in-vscode-a-web-based-svg-graphics-device/)

## About &amp; License

Depends on `cpp11`.

Webserver based on [`CrowCpp/Crow`](<https://github.com/CrowCpp/Crow>).
    
This project is licensed GPL v2.0.

Full copies of the license agreements of vendored components are included in [`./inst/licenses`](https://github.com/nx10/httpgd/tree/master/inst/licenses).
