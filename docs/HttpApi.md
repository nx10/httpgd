# Http API

[httpgd](../README.md) can be called both from [R](RApi.md) and from HTTP.

## API endpoints

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

`http://[host]:[port]/svg?width=400&height=300`

| Key      | Value | 
|----------|-------|
| `width`  | With in pixels. |
| `height` | Height in pixels. |
| `index`  | Plot history index. If omitted, the newest plot will be returned. |
| `token`  | If security tokens are used this should be equal to the previously secret token. |

### Server state

| Field        | Type     | Description |
|--------------|----------|-------------|
| `upid`       | `int`    | Update id. It changes when a the data store receives new information. |
| `hrecording` | `bool`   | Whether the graphics device is recording a plot history. |
| `hsize`      | `int`    | Number of plot history entries. |
