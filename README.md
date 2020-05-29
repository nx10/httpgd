# httpgd

Asynchronous http server graphics device for R.

## Features

* SVG over HTTP.
* Automatic resizing.
* HTML live server.
* Plot history.
* Run multiple servers concurrently.

## Demo

![demo](https://user-images.githubusercontent.com/33600480/83210273-b4c29100-a15a-11ea-8757-052dcf259a1c.gif)

## Installation

```R
devtools::install_github("nx10/httpgd")
```

Depends on `Rcpp`, `later` and `gdtools`.

SVG rendering (especially font rendering) based on `svglite` (https://github.com/r-lib/svglite).

Includes `cpp-httplib` (https://github.com/yhirose/cpp-httplib).

## Usage

Initialize graphics device and start live server with:

```R
httpgd::httpgd()
```

Plot what ever you want.

```R
x = seq(0, 3 * pi, by = 0.1)
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

| Endpoint  | Method | Description |
|-----------|--------|-------------|
| `/`       | `GET`  | Welcome message. |
| `/live`   | `GET`  | Returns HTML/Javascript live server page. |
| `/svg`    | `GET`  | Get rendered SVG. |
| `/state`  | `GET`  | Get current server state. This can be used to check periodically whether a newer SVG can be accessed. |
| `/resize` | `POST` | Trigger graphics device resize. Params are `width` and `height` in pixels. |
| `/next`   | `POST` | Go to the next plot history page. |
| `/prev`   | `POST` | Go to the previous plot history page. |
| `/clear`  | `POST` | Clear plot history. |

#### Server state

| Field       | Type     | Description |
|-------------|----------|-------------|
| `upid`      | `int`    | Update id. |
| `width`     | `double` | Graphics device width (pixel). |
| `height`    | `double` | Graphics device height (pixel). |
| `recording` | `bool`   | Whether the graphics device is recording a plot history. |
| `hsize`     | `int`    | Number of plot history entries. |
| `hindex`    | `int`    | Index of the displayed plot entry. |


## Note

Any advice and suggestions are welcome!

## Planned features

* Use websockets to push changes directly/faster (maybe switch http server backend library).
* Optimization.

## Mac OS

It seems like a recent mac OS update (Catalina) removed shared libraries used by the package `systemfonts` we depend on (see: https://github.com/r-lib/systemfonts/issues/17).

You can install XQuartz as a workaround (https://www.xquartz.org/).