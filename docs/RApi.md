# R API

[httpgd](../README.md) can be called both from R and from [HTTP](HttpApi.md).

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