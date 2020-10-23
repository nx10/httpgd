
// attaches to an image element, captures the next request outcome then destructs itself
class ImageRequestHelper {
    private _success: (this: HTMLImageElement, ev: Event) => any;
    private _error: (this: HTMLImageElement, ev: Event) => any;
    private _image: HTMLImageElement;
    constructor(
        image: HTMLImageElement,
        onSuccess?: (image: HTMLImageElement) => void,
        onError?: (image: HTMLImageElement) => void) {
        this._image = image;
        const _this = this;
        this._success = function success(this: HTMLImageElement, ev: Event): any {
            onSuccess?.(image);
            _this.cleanup();
        }
        this._error = function error(this: HTMLImageElement, ev: Event): any {
            onError?.(image);
            _this.cleanup();
        }
        image.addEventListener('load', this._success);
        image.addEventListener('error', this._error);
    }
    private cleanup(): void {
        this._image.removeEventListener('load', this._success);
        this._image.removeEventListener('error', this._error);
    }
}

class HttpgdParams {
    public index: number = -1;
    public width: number = 0;
    public height: number = 0;
    equals(other: HttpgdParams): boolean {
        return this.index === other.index &&
            Math.abs(this.width - other.width) < 0.1 &&
            Math.abs(this.height - other.height) < 0.1;
    }
}

class HttpgdState {
    public upid: number = -1;
    public hsize: number = 0;
    public active: boolean = true;
    equals(other: HttpgdState): boolean {
        return this.upid === other.upid &&
            this.hsize === other.hsize &&
            this.active === other.active;
    }
}

// Handles httpgd api URLs 
class HttpgdApi {
    public readonly http: string;
    public readonly ws: string;
    public readonly httpSVG: string;
    public readonly httpState: string;
    public readonly httpRemove: string;
    public readonly httpClear: string;
    public readonly httpHeaders: Headers = new Headers();

    public readonly useToken: boolean;
    public readonly token: string;

    public constructor(host: string, token?: string) {
        this.http = 'http://' + host;
        this.ws = 'ws://' + host;
        this.httpSVG = this.http + '/svg';
        this.httpState = this.http + '/state';
        this.httpClear = this.http + '/clear';
        this.httpRemove = this.http + '/remove';
        if (token) {
            this.useToken = true;
            this.token = token;
            this.httpHeaders.set('X-HTTPGD-TOKEN', this.token);
        } else {
            this.useToken = false;
            this.token = '';
        }
    }

    public svgURL(width: number, height: number, index: number, upid: number, force: boolean = false): string {
        // Token needs to be included in query params because request headers can't be set
        // when setting image.src
        // upid is included to avoid caching
        return this.httpSVG +
            '?width=' + Math.round(width) +
            '&height=' + Math.round(height) +
            '&index=' + index +
            (this.useToken ? ('&token=' + this.token) : '') +
            '&upid=' + upid + (force ? 'f' : '');
    }

    public removeURL(index: number) {
        return this.httpRemove + '?index=' + index;
    }
}

const enum HttpgdConnectionMode {
    NONE,
    POLL,
    SLOWPOLL,
    WEBSOCKET
}
// Handles HTTP polling / websocket connection
class HttpgdConnectionManager {
    private static readonly INTERVAL_POLL: number = 500;
    private static readonly INTERVAL_POLL_SLOW: number = 5000;

    private api: HttpgdApi;

    private mode: HttpgdConnectionMode = HttpgdConnectionMode.NONE;
    private allowWebsockets: boolean;

    private socket?: WebSocket;
    private pollHandle?: ReturnType<typeof setInterval>;

    private imageReloading: boolean = false;
    private disconnected: boolean = true;

    public stateCallback?: (remoteState: HttpgdState) => void;
    public stateCallbackRemove?: (remoteState: HttpgdState) => void;
    public connectionCallback?: (disconnected: boolean) => void;

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
                this.pollHandle = setInterval(() => this.poll(), HttpgdConnectionManager.INTERVAL_POLL);
                this.mode = targetMode;
                break;
            case HttpgdConnectionMode.SLOWPOLL:
                console.log("Start SLOWPOLL");
                this.clearWebsocket();
                this.clearPoll();
                this.pollHandle = setInterval(() => this.poll(), HttpgdConnectionManager.INTERVAL_POLL_SLOW);
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

                this.socket = new WebSocket(this.api.ws);
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
        if (this.imageReloading) return; // don't poll while image is being loaded

        // fetch remote state
        fetch(this.api.httpState, {
            headers: this.api.httpHeaders
        })
            .then(res => res.json())
            .then((remoteState: HttpgdState) => {
                this.setDisconnected(false);
                // todo: if slow poll upgrade to fast poll
                if (this.imageReloading) return;
                this.stateCallback?.(remoteState);
            }).catch((e) => {
                console.warn(e);
                this.setDisconnected(true);
            });
    }

    private onWsMessage(message: string): void {
        if (message.startsWith('{')) {
            const remoteState = JSON.parse(message);
            this.stateCallback?.(remoteState);
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
            this.connectionCallback?.(disconnected);
        }
    }

    public loadImage(width: number, height: number, index: number, upid: number, force?: boolean, image?: HTMLImageElement): void {
        if (!image) return;

        new ImageRequestHelper(
            image,
            (image) => {
                //console.log("SVG loaded!");
                this.imageReloading = false;
            },
            (image) => {
                console.warn("SVG load failed!");
                this.imageReloading = false;
            }
        );

        image.src = this.api.svgURL(width, height, index, upid, force);
    }

    public clearPlots(): void {
        fetch(this.api.httpClear, {
            headers: this.api.httpHeaders
        }).then(res => res.json())
            .then((remoteState: HttpgdState) => {
                this.stateCallback?.(remoteState);
            });
    }
    public removePlot(index: number): void {
        fetch(this.api.removeURL(index), {
            headers: this.api.httpHeaders
        }).then(res => res.json())
            .then((remoteState: HttpgdState) => {
                this.stateCallbackRemove?.(remoteState);
            });
    }

    public svgURL(width: number, height: number, index: number, upid: number): string {
        return this.api.svgURL(width, height, index, upid);
    }
}

class HttpgdViewer {
    static readonly COOLDOWN_RESIZE: number = 500;
    static readonly SCALE_DEFAULT: number = 0.8;
    static readonly SCALE_STEP: number = HttpgdViewer.SCALE_DEFAULT / 12.0;

    private state: HttpgdState = new HttpgdState(); // state of last update
    private params: HttpgdParams = new HttpgdParams(); // can be edited
    private plotParams: HttpgdParams = new HttpgdParams(); // params at last request
    private scale: number = HttpgdViewer.SCALE_DEFAULT; // zoom level

    private connection: HttpgdConnectionManager;
    private deviceActive: boolean = true;
    private image?: HTMLImageElement = undefined;

    public onDeviceActiveChange?: (deviceActive: boolean) => void;
    public onDisconnectedChange?: (disconnected: boolean) => void;
    public onIndexStringChange?: (indexString: string) => void;
    public onZoomStringChange?: (zoomString: string) => void;

    public constructor(host: string, token?: string, allowWebsockets?: boolean) {
        this.connection = new HttpgdConnectionManager(host, token, allowWebsockets);
        this.connection.stateCallback = (remoteState: HttpgdState) => this.serverChanges(remoteState);
        this.connection.stateCallbackRemove = (remoteState: HttpgdState) => {
            this.params.index = Math.max(-1, this.params.index - 1);
            this.serverChanges(remoteState);
        };
        this.connection.connectionCallback = (disconnected: boolean) => this.onDisconnectedChange?.(disconnected);
    }

    public init(image: HTMLImageElement): void {
        this.image = image;

        this.connection.open();
        this.checkResize();

        // Force reload on visibility change
        // Firefox otherwise shows a blank screen on tab change 
        document.addEventListener('visibilitychange', () => {
            if (!document.hidden) {
                this.loadImage(true);
            }
        }, false);

        this.onIndexStringChange?.(this.getIndexString());
        this.onZoomStringChange?.(this.getZoomString());
    }

    private loadImage(force?: boolean) {
        this.connection.loadImage(
            this.params.width,
            this.params.height,
            this.params.index,
            this.state.upid,
            force,
            this.image);
    }

    // checks if there were server side changes
    private serverChanges(remoteState: HttpgdState): void {
        this.setDeviceActive(!remoteState.active);
        let needsReload: boolean = false;
        if (this.diffServer(remoteState)) {
            Object.assign(this.state, remoteState);
            this.params.index = -1; // fetch newest
            needsReload = true;
        }
        if (this.diffClient()) {
            Object.assign(this.plotParams, this.params);
            needsReload = true;
        }
        // request new image
        if (needsReload) {
            this.onIndexStringChange?.(this.getIndexString());
            this.loadImage();
        }
    }

    // checks if there were client side changes
    private clientChanges(): void {
        if (this.diffClient()) {
            Object.assign(this.plotParams, this.params);
            this.onIndexStringChange?.(this.getIndexString());
            this.loadImage();
        }
    }

    // Server side changes?
    private diffServer(remoteState: HttpgdState): boolean {
        console.log("diff server: "+ !this.state.equals(remoteState));
        return !this.state.equals(remoteState);
    }

    // Client side changes?
    private diffClient(): boolean {
        console.log("diff client: "+ !this.params.equals(this.plotParams));
        return !this.params.equals(this.plotParams);
    }

    private setDeviceActive(active: boolean): void {
        if (this.deviceActive != active) {
            this.deviceActive = active;
            this.onDeviceActiveChange?.(active);
        }
    }


    // User interaction
    public zoomIn(): void {
        if (this.scale - HttpgdViewer.SCALE_STEP > 0.05) {
            this.scale -= HttpgdViewer.SCALE_STEP;
        }
        this.onZoomStringChange?.(this.getZoomString());
        this.checkResize();
    }
    public zoomOut(): void {
        this.scale += HttpgdViewer.SCALE_STEP;
        this.onZoomStringChange?.(this.getZoomString());
        this.checkResize();
    }
    public zoomReset(): void {
        this.scale = HttpgdViewer.SCALE_DEFAULT;
        this.onZoomStringChange?.(this.getZoomString());
        this.checkResize();
    }
    public getZoomString(): string {
        return Math.ceil(HttpgdViewer.SCALE_DEFAULT / this.scale * 100) + '%';
    }
    public navPrevious(): void {
        if (this.params.index > 0) {
            this.params.index -= 1;
        } else if (this.state.hsize >= 2) {
            this.params.index = this.state.hsize - (this.params.index == -1 ? 2 : 1);
        }
        this.onIndexStringChange?.(this.getIndexString());
        this.clientChanges();
    }
    public navNext(): void {
        if (this.params.index >= 0 && this.params.index < this.state.hsize - 1) {
            this.params.index += 1;
        } else {
            this.params.index = 0;
        }
        this.onIndexStringChange?.(this.getIndexString());
        this.clientChanges();
    }
    public navNewest(): void {
        this.params.index = -1;
        this.onIndexStringChange?.(this.getIndexString());
        this.clientChanges();
    }
    public getIndexString(): string {
        return (this.params.index === -1 ? this.state.hsize : (this.params.index + 1)) + '/' + this.state.hsize;
    }
    public navClear(): void {
        this.connection.clearPlots();
    }
    public navRemove(): void {
        this.connection.removePlot(this.params.index);
    }
    private svgURL(): string {
        return this.connection.svgURL(
            this.plotParams.width,
            this.plotParams.height,
            this.plotParams.index,
            this.state.upid);
    }
    private static downloadURL(url: string, filename?: string) {
        const dl = document.createElement('a');
        dl.href = url;
        if (filename) { dl.download = filename; }
        document.body.appendChild(dl);
        dl.click();
        document.body.removeChild(dl);
    }
    public downloadPlot(filename?: string) {
        HttpgdViewer.downloadURL(
            this.svgURL(),
            filename ? filename : 'plot.svg');
    }
    public downloadPlotPNG() {
        const canvas = document.createElement('canvas');
        document.body.appendChild(canvas);
        canvas.width = this.plotParams.width;
        canvas.height = this.plotParams.height;
        const ctx = canvas.getContext('2d');
        if (!ctx) return;
        const img = new Image();
        img.crossOrigin = "anonymous";
        img.onload = () => {
            ctx.drawImage(img, 0, 0);
            var imgURI = canvas
                .toDataURL('image/png')
                .replace('image/png', 'image/octet-stream');
            HttpgdViewer.downloadURL(imgURI, 'plot.png');
        };
        img.src = this.svgURL();
        document.body.removeChild(canvas);
    }

    public checkResize() {
        if (!this.image) return;
        const rect = this.image.getBoundingClientRect();
        this.params.width = rect.width * this.scale;
        this.params.height = rect.height * this.scale;
        this.clientChanges();
    }

    // this is called by window.addEventListener('resize', ...)
    private resizeBlocked: boolean = false;
    public resize() {
        if (this.resizeBlocked) return;
        this.resizeBlocked = true;
        setTimeout(() => {
            this.checkResize();
            this.resizeBlocked = false;
        }, HttpgdViewer.COOLDOWN_RESIZE);
    }
}