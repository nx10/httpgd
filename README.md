# httpgd

Http server graphics device for R.

## Installation

```R
install_github("nx10/httpgd")
```

This depends on `Rcpp`, `RcppThread` and `gdtools`.

Heavily based on `svglite` (https://github.com/r-lib/svglite).

Includes `cpp-httplib` (https://github.com/yhirose/cpp-httplib).

## Usage

Initialize graphics device and start live server with:

```R
httpgd::httpgd()
```

Plot what ever you want.

```R
x = seq(0, 3*pi, by = 0.1)
plot(x, sin(x), type = "l")
```

Every plotting library should work.

```R
library(ggplot2)

ggplot(mpg, aes(displ, hwy, colour = class)) +
  geom_point()

```

The live server should refresh automatically and detect window size changes.

Stop the server with:

```R
dev.off()
```

### HTTP API

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/`        | `GET`  | Server status |
| `/live`    | `GET`  | Returns HTML/Javascript live server page. |
| `/svg`     | `GET`  | Get rendered SVG |
| `/check`   | `GET`  | Get current update id.<br/> This can be checked periodically if a newer SVG can be accessed |
| `/rs`      | `POST` | Trigger graphics device resize. Params are width `w` and height `h` in pixels. |


## Limitations (Need help!)

I can't get refreshes from the server thread to the R session working. For this reason the user hat to manually call a function after resize while updates from R to the servers are no problem.
It has to be possible to asynchronously trigger an event in R somehow.

The code is very experimental, I am not a very experienced C++ programmer and am not too familiar with the R/C++ API. Any advice and suggestions are welcome!


