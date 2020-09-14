# Web API

[httpgd](../README.md) can be accessed both from [R](RApi.md) and from HTTP/websockets.

## HTTP API endpoints

| Endpoint  | Method | Description |
|-----------|--------|-------------|
| `/`       | `GET`  | Welcome message. |
| `/live`   | `GET`  | Returns HTML/Javascript live server page. |
| `/svg`    | `GET`  | Get rendered SVG. Query parameters are listed below. |
| `/state`  | `GET`  | Get current server state. This can be used to check if there were new draw calls in R. |
| `/clear`  | `GET`  | Clear plot history. |
| `/remove`  | `GET`  | Remove a single plot from the history. Index specified with query param `index`. |

### Query parameter

A query looks like this:

```
http://[host]:[port]/svg?width=400&height=300
```

| Key      | Value | 
|----------|-------|
| `width`  | With in pixels. |
| `height` | Height in pixels. |
| `index`  | Plot history index. If omitted, the newest plot will be returned. |
| `token`  | If enabled, security tokens need to be attached to every request. (The `X-HTTPGD-TOKEN` header can be set alternatively.) |

### Server state

The server state will be returned as JSON.

Example:

```JSON
{ "upid": 1234, "hsize": 12 }
```

| Field        | Type     | Description |
|--------------|----------|-------------|
| `upid`       | `int`    | Update id. Changes when the data store receives new information. |
| `hsize`      | `int`    | Number of plot history entries. |

## Websockets

httpgd accepts websocket connections on the same port as the HTTP server. [Server state](#Server-state) changes will be broadcasted immediately to all connected clients in JSON format. 

If websockets are unavailable we recommend to detect state changes via polling `GET` `/state`.