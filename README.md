# `httpgd` <img src="man/figures/httpgd_logo.svg" align="right" height=250/>

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
  * Stateless asynchronous HTTP/WebSocket API
  * In-memory access to rendered plots
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

See [system requirements](https://nx10.github.io/httpgd/articles/a00_installation.html#system-requirements) for troubleshooting.


### Documentation

- For users:
  - [How to get started](https://nx10.github.io/httpgd/articles/a01_how-to-get-started.html)
  - [Function reference](https://nx10.github.io/httpgd/reference/index.html)
  <!-- - [Benchmarks](https://nx10.github.io/httpgd/articles/c02_benchmarks.html) -->
  - IDEs &amp; evironments:
    - [VS Code](https://nx10.github.io/httpgd/articles/b01_vscode.html)
    - [RStudio](https://nx10.github.io/httpgd/articles/b02_rstudio.html)
    - [Docker](https://nx10.github.io/httpgd/articles/b03_docker.html)
- For developers:
  - [httpgd API](https://nx10.github.io/httpgd/articles/c01_httpgd-api.html)
  <!-- - [Technical documentation](https://nx10.github.io/httpgd/articles/c03_technical-docs.html) -->




## Contributions welcome!

The various components of `httpgd` are written in C++, R and TypeScript. We welcome contributions of any kind.

Other areas in need of improvement are testing and documentation.

## Links &amp; Articles

- [Using httpgd in VSCode: A web-based SVG graphics device](https://renkun.me/2020/06/16/using-httpgd-in-vscode-a-web-based-svg-graphics-device/)

## About &amp; License

Depends on `cpp11`, `later` and `systemfonts`.

Webserver based on [`Boost/Beast`](<https://github.com/boostorg/beast>) included in the `BH` package.
    
Much of the font handling and SVG rendering code is modified code from the excellent [`svglite`](<https://github.com/r-lib/svglite>) package.

This project is licensed GPL v2.0.

It includes parts of [`svglite`](<https://github.com/r-lib/svglite>) (GPL &geq; 2), [`Belle`](https://github.com/octobanana/belle) (MIT) and [`fmt`](https://github.com/fmtlib/fmt) (MIT). The HTML client includes [Material Design icons by Google](https://github.com/google/material-design-icons) which are licensed under the [Apache License Version 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt).

Full copies of the license agreements used by these components are included in [`./inst/licenses`](https://github.com/nx10/httpgd/tree/master/inst/licenses).