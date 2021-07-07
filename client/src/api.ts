import {
    HttpgdBackend, HttpgdStateResponse, HttpgdPlotsResponse, HttpgdRenderersResponse,
    HttpgdPlotRequest, HttpgdRemoveRequest
} from './types'

const URL_HTTP = 'http://';
const URL_WS = 'ws://';
const URL_STATE = '/state';
const URL_CLEAR = '/clear';
const URL_REMOVE = '/remove';
const URL_PLOTS = '/plots';
const URL_PLOT = '/plot';
const URL_RENDERERS = '/renderers';
const HEADER_TOKEN = 'X-HTTPGD-TOKEN';

function make_headers(b: HttpgdBackend): Headers {
    const headers = new Headers();
    if (b.token) {
        headers.set(HEADER_TOKEN, this.token);
    }
    return headers;
}

async function fetch_url<ResponseType>(b: HttpgdBackend, url: string): Promise<ResponseType> {
    const res = await fetch(url, {
        headers: make_headers(b)
    });
    return await (res.json() as Promise<ResponseType>);
}

// API -------------------------------------------------------------------------------------------

export function url_websocket(b: HttpgdBackend): string {
    return URL_WS + b.host;
}

export function new_websocket(b: HttpgdBackend): WebSocket {
    return new WebSocket(url_websocket(b));
}

// /state

export function url_state(b: HttpgdBackend): string {
    return URL_HTTP + b.host + URL_STATE;
}

export function fetch_state(b: HttpgdBackend): Promise<HttpgdStateResponse> {
    return fetch_url<HttpgdStateResponse>(b, url_state(b));
}

// /clear

export function url_clear(b: HttpgdBackend): string {
    return URL_HTTP + b.host + URL_CLEAR;
}

export function fetch_clear(b: HttpgdBackend): Promise<HttpgdStateResponse> {
    return fetch_url<HttpgdStateResponse>(b, url_clear(b));
}

// /renderers

export function url_renderers(b: HttpgdBackend): string {
    return URL_HTTP + b.host + URL_RENDERERS;
}

export function fetch_renderers(b: HttpgdBackend): Promise<HttpgdRenderersResponse> {
    return fetch_url<HttpgdRenderersResponse>(b, url_renderers(b));
}

// /plots

export function url_plots(b: HttpgdBackend): string {
    return URL_HTTP + b.host + URL_PLOTS;
}

export function fetch_plots(b: HttpgdBackend): Promise<HttpgdPlotsResponse> {
    return fetch_url<HttpgdPlotsResponse>(b, url_plots(b));
}

// /plot

export function url_plot(b: HttpgdBackend, r: HttpgdPlotRequest, includeToken?: boolean, cachestr?: string): string {
    const url = new URL(URL_HTTP + b.host + URL_PLOT);
    if (r.id) url.searchParams.append('id', r.id);
    if (r.renderer) url.searchParams.append('renderer', r.renderer);
    if (r.width) url.searchParams.append('width', Math.round(r.width).toString());
    if (r.height) url.searchParams.append('height', Math.round(r.height).toString());
    if (r.zoom) url.searchParams.append('zoom', r.zoom.toString());
    if (r.download) url.searchParams.append('download', r.download);
    // In some cases token needs to be included in query params because request headers can't be set (e.g. img.src)
    if (includeToken && b.token) url.searchParams.append('token', b.token);
    if (cachestr) url.searchParams.append('c', cachestr);
    return url.href;
}

// eslint-disable-next-line @typescript-eslint/no-explicit-any
export function fetch_plot(b: HttpgdBackend, r: HttpgdPlotRequest): Promise<any> {
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    return fetch_url<any>(b, url_plot(b, r));
}

// /remove

export function url_remove(b: HttpgdBackend, r: HttpgdRemoveRequest): string {
    const url = new URL(URL_HTTP + b.host + URL_REMOVE);
    url.searchParams.append('id', r.id);
    return url.href;
}

export function fetch_remove(b: HttpgdBackend, r: HttpgdRemoveRequest): Promise<HttpgdStateResponse> {
    return fetch_url<HttpgdStateResponse>(b, url_remove(b, r));
}