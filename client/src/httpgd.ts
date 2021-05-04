const plot_none_uri = require('./assets/plot-none.svg')

// httpgd connection ----------------------------------------------------------

export interface HttpgdState {
    upid: number,
    hsize: number,
    active: boolean
}

export interface HttpgdId {
    id: string
}

export interface HttpgdPlots {
    state: HttpgdState,
    plots: HttpgdId[]
}

export interface HttpgdRenderer {
    id: string, 
    mime: string, 
    ext: string, 
    name: string, 
    type: string, 
    bin: boolean
}

export interface HttpgdRenderersResponse {
    renderers: HttpgdRenderer[]
}

export class HttpgdApi {
    private readonly http: string;
    private readonly ws: string;
    private readonly httpSVG: string;
    private readonly httpState: string;
    private readonly httpRemove: string;
    private readonly httpClear: string;
    private readonly httpPlots: string;
    private readonly httpPlot: string;
    private readonly httpRenderers: string;
    private readonly httpHeaders: Headers = new Headers();

    private readonly useToken: boolean;
    private readonly token: string;

    public constructor(host: string, token?: string) {
        this.http = 'http://' + host;
        this.ws = 'ws://' + host;
        this.httpSVG = this.http + '/svg';
        this.httpState = this.http + '/state';
        this.httpClear = this.http + '/clear';
        this.httpRemove = this.http + '/remove';
        this.httpPlots = this.http + '/plots';
        this.httpPlot = this.http + '/plot';
        this.httpRenderers = this.http + '/renderers';
        if (token) {
            this.useToken = true;
            this.token = token;
            this.httpHeaders.set('X-HTTPGD-TOKEN', this.token);
        } else {
            this.useToken = false;
            this.token = '';
        }
    }

    public svg_index(index: number, width?: number, height?: number, c?: string): URL {
        const url = this.svg_ext(width, height, c);
        url.searchParams.append('index', index.toString());
        return url;
    }

    public svg_id(id: string, width?: number, height?: number, c?: string): URL {
        const url = this.svg_ext(width, height, c);
        url.searchParams.append('id', id);
        return url;
    }

    private svg_ext(width?: number, height?: number, c?: string): URL {
        const url = new URL(this.httpSVG);
        if (width) url.searchParams.append('width', Math.round(width).toString());
        if (height) url.searchParams.append('height', Math.round(height).toString());
        // Token needs to be included in query params because request headers can't be set
        // when setting image.src
        // upid is included to avoid caching
        if (this.useToken) url.searchParams.append('token', this.token);
        if (c) url.searchParams.append('c', c);
        return url;
    }

    private remove_index(index: number): URL {
        const url = new URL(this.httpRemove);
        url.searchParams.append('index', index.toString());
        return url;
    }

    public async get_remove_index(index: number): Promise<any> {
        const res = await fetch(this.remove_index(index).href, {
            headers: this.httpHeaders
        });
        return res;
    }

    private remove_id(id: string): URL {
        const url = new URL(this.httpRemove);
        url.searchParams.append('id', id);
        return url;
    }

    public async get_remove_id(id: string): Promise<any> {
        const res = await fetch(this.remove_id(id).href, {
            headers: this.httpHeaders
        });
        return res;
    }

    public async get_plots(): Promise<HttpgdPlots> {
        const res = await fetch(this.httpPlots, {
            headers: this.httpHeaders
        });
        return await (res.json() as Promise<HttpgdPlots>);
    }

    public async get_clear(): Promise<any> {
        const res = await fetch(this.httpClear, {
            headers: this.httpHeaders
        });
        return res;
    }

    public async get_state(): Promise<HttpgdState> {
        const res = await fetch(this.httpState, {
            headers: this.httpHeaders
        });
        return await (res.json() as Promise<HttpgdState>);
    }

    public async get_renderers(): Promise<HttpgdRenderersResponse> {
        const res = await fetch(this.httpRenderers, {
            headers: this.httpHeaders
        });
        return await (res.json() as Promise<HttpgdRenderersResponse>);
    }

    public plot_id(id: string, renderer: string, width?: number, height?: number, c?: string, download?: string): URL {
        const url = new URL(this.httpPlot);
        url.searchParams.append('id', id);
        url.searchParams.append('renderer', renderer);
        if (width) url.searchParams.append('width', Math.round(width).toString());
        if (height) url.searchParams.append('height', Math.round(height).toString());
        if (download) url.searchParams.append('download', download);
        // Token needs to be included in query params because request headers can't be set
        // when setting image.src
        // upid is included to avoid caching
        if (this.useToken) url.searchParams.append('token', this.token);
        if (c) url.searchParams.append('c', c);
        return url;
    }

    public new_websocket(): WebSocket {
        return new WebSocket(this.ws);
    }
}

export const enum HttpgdConnectionMode {
    NONE,
    POLL,
    SLOWPOLL,
    WEBSOCKET
}
// Handles HTTP polling / websocket connection
export class HttpgdConnection {
    private static readonly INTERVAL_POLL: number = 500;
    private static readonly INTERVAL_POLL_SLOW: number = 5000;

    public api: HttpgdApi;

    private mode: HttpgdConnectionMode = HttpgdConnectionMode.NONE;
    private allowWebsockets: boolean;

    private socket?: WebSocket;
    private pollHandle?: ReturnType<typeof setInterval>;

    private pausePoll: boolean = false;
    private disconnected: boolean = true;

    private lastState?: HttpgdState;

    public remoteStateChanged?: (newState: HttpgdState) => void;
    public connectionChanged?: (disconnected: boolean) => void;

    public constructor(host: string, token?: string, allowWebsockets?: boolean) {
        this.api = new HttpgdApi(host, token);
        this.allowWebsockets = allowWebsockets ? allowWebsockets : false;
    }

    public open(): void {
        if (this.mode != HttpgdConnectionMode.NONE) return;
        this.start(HttpgdConnectionMode.WEBSOCKET);
    }

    public close(): void {
        if (this.mode == HttpgdConnectionMode.NONE) return;
        this.start(HttpgdConnectionMode.NONE);
    }

    private start(targetMode: HttpgdConnectionMode): void {
        if (this.mode == targetMode) return;

        switch (targetMode) {
            case HttpgdConnectionMode.POLL:
                console.log("Start POLL");
                this.clearWebsocket();
                this.clearPoll();
                this.pollHandle = setInterval(() => this.poll(), HttpgdConnection.INTERVAL_POLL);
                this.mode = targetMode;
                break;
            case HttpgdConnectionMode.SLOWPOLL:
                console.log("Start SLOWPOLL");
                this.clearWebsocket();
                this.clearPoll();
                this.pollHandle = setInterval(() => this.poll(), HttpgdConnection.INTERVAL_POLL_SLOW);
                this.mode = targetMode;
                break;
            case HttpgdConnectionMode.WEBSOCKET:
                if (!this.allowWebsockets) {
                    this.start(HttpgdConnectionMode.POLL);
                    break;
                }
                console.log("Start WEBSOCKET");
                this.clearPoll();
                this.clearWebsocket();

                this.socket = this.api.new_websocket();
                this.socket.onmessage = (ev) => this.onWsMessage(ev.data);
                this.socket.onopen = () => this.onWsOpen();
                this.socket.onclose = () => this.onWsClose();
                this.socket.onerror = () => console.log('Websocket error');
                this.mode = targetMode;
                this.poll(); // get initial state
                break;
            case HttpgdConnectionMode.NONE:
                this.clearWebsocket();
                this.clearPoll();
                this.mode = targetMode;
                break;
            default:
                break;
        }

    }

    private clearPoll() {
        if (this.pollHandle) {
            clearInterval(this.pollHandle);
        }
    }

    private clearWebsocket() {
        if (this.socket) {
            this.socket.onclose = () => { };
            this.socket.close();
        }
    }

    private poll(): void {
        if (this.pausePoll) return;
        this.api.get_state().then((remoteState: HttpgdState) => {
            this.setDisconnected(false);
            if (this.mode === HttpgdConnectionMode.SLOWPOLL) this.start(HttpgdConnectionMode.WEBSOCKET); // reconnect
            if (this.pausePoll) return;
            this.checkState(remoteState);
        }).catch((e) => {
            console.warn(e);
            this.setDisconnected(true);
        });
    }

    private onWsMessage(message: string): void {
        if (message.startsWith('{')) {
            const remoteState = JSON.parse(message) as HttpgdState;
            this.checkState(remoteState);
        } else {
            console.log("Unknown WS message: " + message);
        }
    }
    private onWsClose(): void {
        console.log('Websocket closed');
        this.setDisconnected(true);
    }
    private onWsOpen(): void {
        console.log('Websocket opened');
        this.setDisconnected(false);
    }

    private setDisconnected(disconnected: boolean): void {
        if (this.disconnected != disconnected) {
            this.disconnected = disconnected;
            if (this.disconnected) {
                this.start(HttpgdConnectionMode.SLOWPOLL);
            } else {
                this.start(HttpgdConnectionMode.WEBSOCKET);
            }
            this.connectionChanged?.(disconnected);
        }
    }

    private checkState(remoteState: HttpgdState): void {
        if (
            (!this.lastState) ||
            (this.lastState.active !== remoteState.active) ||
            (this.lastState.hsize !== remoteState.hsize) ||
            (this.lastState.upid !== remoteState.upid)
        ) {
            this.lastState = remoteState;
            this.remoteStateChanged?.(remoteState);
        }
    }
}

// httpgd viewer --------------------------------------------------------------

export class HttpgdNavigator {
    private data?: HttpgdPlots;
    private index: number = -1;
    private width: number = 0;
    private height: number = 0;

    private last_id: string = "";
    private last_width: number = 0;
    private last_height: number = 0;

    public navigate(offset: number): void {
        if (!this.data) return;
        this.index = (this.data.plots.length + this.index + offset) % this.data.plots.length;
    }

    public jump(index: number): void {
        if (!this.data) return;
        this.index = (this.data.plots.length + index) % this.data.plots.length;
    }

    public jump_id(id: string): void {
        if (!this.data) return;
        for (let i = 0; i < this.data.plots.length; i++) {
            if (id === this.data.plots[i].id) {
                this.index = i;
                break;
            }
        }
    }

    public resize(width: number, height: number): void {
        this.width = width;
        this.height = height;
    }

    public next(api: HttpgdApi, c?: string): string | undefined {
        if (!this.data || this.data.plots.length == 0) return plot_none_uri;
        if ((this.last_id !== this.data.plots[this.index].id) ||
            (Math.abs(this.last_width - this.width) > 0.1) ||
            (Math.abs(this.last_height - this.height) > 0.1))
            return api.svg_id(this.data.plots[this.index].id, this.width, this.height, c).href;
        return undefined;
    }

    public update(data: HttpgdPlots) {
        this.data = data;
        this.index = data.plots.length - 1;
    }

    public id(): string | undefined {
        if (!this.data || this.data.plots.length == 0) return undefined;
        return this.data.plots[this.index].id;
    }

    public indexStr(): string {
        if (!this.data) return '0/0';
        return Math.max(0, this.index + 1) + '/' + this.data.plots.length;
    }
}

