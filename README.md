# httpgd

(Experimental) Http server graphics device for R.

## Installation

```R
devtools::install_github("nx10/httpgd")
```

Depends on `Rcpp`, `later` and `gdtools`.

SVG rendering heavily based on `svglite` (https://github.com/r-lib/svglite).

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


## Note

The code is very experimental. Any advice and suggestions are welcome!

## ToDo

httpgd is under active development.

* Implement the rest of the `svglite` functionality (clipping)
* Use websockets for updates
* Plot history
* Clean up code
* Optimization

## Mac OS

It seems like a recent mac OS update (Catalina) removed shared libraries used by the package `systemfonts` we depend on (see: https://github.com/r-lib/systemfonts/issues/17).

You can install XQuartz as a workaround (https://www.xquartz.org/).