# R API

[httpgd](../README.md) can be accessed both from R and from [HTTP](HttpApi.md).

## Render SVG

```R
httpgdSVG(page, width, height, ...)
```

## Remove pages

```R
httpgdRemove(page, ...) # Remove a single page
httpgdClear(...) # Clear all pages
```

## Server state

```R
httpgdState()
```

This will return the current server state as a R list:

```R
$host
[1] "127.0.0.1"

$port
[1] 63072

$token
[1] "5hC6D3LC"

$hsize
[1] 0

$upid
[1] 0
```
`hsize` is the number of pages in the plot history.
`upid` is the update id. It changes when a the data store receives new information.

## Security

A security token can be set when starting the device:

```R
httpgd(..., token = "secret")
```

When set, each API request has to include this token inside the header `X-HTTPGD-TOKEN` or as a query param `?token=secret`.
`token` is by default set to `TRUE` to generate a random 8 character alphanumeric token. If it is set to a number, a random token of that length will be generated. `FALSE` deactivates the security token.

CORS is off by default but can be enabled on startup:

```R
httpgd(..., cors = TRUE)
```
