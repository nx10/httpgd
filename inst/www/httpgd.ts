
const CHECK_INTERVAL = 500;
const SCALE_DEFAULT = 0.8;
const SCALE_STEP = SCALE_DEFAULT / 12;

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
    equals(other: HttpgdState): boolean {
        return this.upid === other.upid &&
            this.hsize === other.hsize;
    }
}

class HttpgdViewer {
    private useWebsockets: boolean = false;
    private socket?: WebSocket;
    private apiAddress: string;
    private apiWebsocket: string;
    private apiSVG: string;
    private apiState: string;
    private apiRemove: string;
    private apiClear: string;
    private apiHeaders: Headers = new Headers();

    private pollIntervall: number = CHECK_INTERVAL;
    private pollIntervallSlow: number = CHECK_INTERVAL * 10;
    private pollHandle?: ReturnType<typeof setInterval>;

    //private recordHistory: boolean;
    private useToken: boolean;
    private token: string;

    private scale: number = SCALE_DEFAULT; // zoom level

    private disconnected: boolean = false;
    private canResize: boolean = true;
    private resizeCooldown: number = CHECK_INTERVAL;

    private state: HttpgdState = new HttpgdState(); // state of last update

    private params: HttpgdParams = new HttpgdParams(); // can be edited
    private plotParams: HttpgdParams = new HttpgdParams(); // params at last request

    private reloading: boolean = false;

    private image?: HTMLImageElement = undefined;

    public onDisconnectedChange?: (disconnected: boolean) => void;
    public onIndexStringChange?: (indexString: string) => void;
    public onZoomStringChange?: (zoomString: string) => void;

    public constructor(host: string, token?: string, useWebsockets?: boolean) {
        this.apiAddress = 'http://' + host;
        this.apiWebsocket = 'ws://' + host;
        this.apiSVG = this.apiAddress + '/svg';
        this.apiState = this.apiAddress + '/state';
        this.apiClear = this.apiAddress + '/clear';
        this.apiRemove = this.apiAddress + '/remove';
        if (token) {
            this.useToken = true;
            this.token = token;
            this.apiHeaders.set('X-HTTPGD-TOKEN', this.token);
        } else {
            this.useToken = false;
            this.token = '';
        }
        this.useWebsockets = (useWebsockets ? useWebsockets : false);
    }

    public init(image?: HTMLImageElement): void {
        if (image) {
            this.image = image;
            this.image.addEventListener('load', () => this.imageReloaded(true));
            this.image.addEventListener('error', () => this.imageReloaded(false));
        }

        this.resize();

        if (this.useWebsockets) {
            this.socket = new WebSocket(this.apiWebsocket);
            this.socket.onmessage = (ev) => this.onWsMessage(ev.data);
            this.socket.onopen = () => this.onWsOpen();
            this.socket.onclose = () => this.onWsClose();
            this.socket.onerror = () => console.log('Websocket error');
        } else {
            this.startPolling();
        }

        // Force reload on visibility change
        // Firefox otherwise shows a blank screen on tab change 
        document.addEventListener('visibilitychange', () => {
            if (!document.hidden) {
                this.reloadImage(true);
            }
        }, false);

        this.notifyIndex();
        this.notifyZoom();
        this.notifyDisconnected();
    }

    private onWsMessage(message: string): void {
        if (message.startsWith('{')) {
            const remoteState = JSON.parse(message);
            this.compareRemote(remoteState);
        }
    }
    private onWsClose(): void {
        console.log('Websocket closed');
        this.setDisconnected(true);
    }
    private onWsOpen(): void {
        console.log('Websocket opened');
        this.setDisconnected(true);
    }

    private startPolling(): void {
        if (this.pollHandle) {
            clearInterval(this.pollHandle);
        }
        this.pollHandle = setInterval(() => this.poll(), this.pollIntervall);
    }

    // callbacks
    private notifyZoom(): void {
        if (this.onZoomStringChange) {
            this.onZoomStringChange(this.getZoomString());
        }
    }
    private notifyIndex(): void {
        if (this.onIndexStringChange) {
            this.onIndexStringChange(this.getIndexString());
        }
    }
    private notifyDisconnected(): void {
        if (this.onDisconnectedChange) {
            this.onDisconnectedChange(this.disconnected);
        }
    }

    // User interaction
    public zoomIn(): void {
        if (this.scale - SCALE_STEP > 0) {
            this.scale -= SCALE_STEP;
        }
        this.notifyZoom();
        this.resize();
    }
    public zoomOut(): void {
        this.scale += SCALE_STEP;
        this.notifyZoom();
        this.resize();
    }
    public zoomReset(): void {
        this.scale = SCALE_DEFAULT;
        this.notifyZoom();
        this.resize();
    }
    public getZoomString(): string {
        return Math.ceil(SCALE_DEFAULT / this.scale * 100) + '%';
    }
    public navPrevious(): void {
        if (this.params.index > 0) {
            this.params.index -= 1;
        } else if (this.state.hsize >= 2) {
            this.params.index = this.state.hsize - (this.params.index == -1 ? 2 : 1);
        }
        this.notifyIndex();
        this.poll();
    }
    public navNext(): void {
        if (this.params.index >= 0 && this.params.index < this.state.hsize - 1) {
            this.params.index += 1;
        } else {
            this.params.index = 0;
        }
        this.notifyIndex();
        this.poll();
    }
    public navNewest(): void {
        this.params.index = -1;
        this.notifyIndex();
        this.poll();
    }
    public getIndexString(): string {
        return (this.params.index === -1 ? this.state.hsize : (this.params.index + 1)) + '/' + this.state.hsize;
    }
    public navClear(): void {
        fetch(this.apiClear, {
            headers: this.apiHeaders
        }).then(res => res.json())
            .then((remoteState: HttpgdState) => {
                // todo clear image (?)
                if (this.reloading) return;
                this.compareRemote(remoteState);
            });
    }
    public navRemove(): void {
        fetch(this.removeURL(this.params), {
            headers: this.apiHeaders
        }).then(res => res.json())
            .then((remoteState: HttpgdState) => {
                this.params.index = Math.min(-1, this.params.index - 1);
                // todo: refresh image
                this.compareRemote(remoteState);
            });
    }
    public downloadPlot(filename?: string) {
        const dl = document.createElement('a');
        dl.href = this.svgURL(this.plotParams, this.state);
        dl.download = filename ? filename : 'plot.svg';
        document.body.appendChild(dl);
        dl.click();
        document.body.removeChild(dl);
    }

    public resize() {
        if (this.image) {
            const rect = this.image.getBoundingClientRect();
            this.params.width = rect.width * this.scale;
            this.params.height = rect.height * this.scale;
        }

        // trigger update when not in cooldown
        if (!this.canResize || this.reloading) return;
        this.canResize = false;
        this.poll();
        setTimeout(() => this.canResize = true, this.resizeCooldown);
    }

    private setDisconnected(disconnected: boolean): void {
        if (this.disconnected != disconnected) {
            this.disconnected = disconnected;
            clearInterval(this.pollHandle);
            this.pollHandle = setInterval(() => this.poll(), this.disconnected ? this.pollIntervallSlow : this.pollIntervall);
            this.notifyDisconnected();
        }
    }

    private svgURL(params: HttpgdParams, state: HttpgdState, force: boolean = false): string {
        // Token needs to be included in query params because request headers can't be set
        // when setting image.src
        // upid is included to avoid caching
        return this.apiSVG +
            '?width=' + Math.round(params.width) +
            '&height=' + Math.round(params.height) +
            '&index=' + params.index +
            (this.useToken ? ('&token=' + this.token) : '') +
            '&upid=' + state.upid + (force ? 'f' : '');
    }
    private removeURL(params: HttpgdParams) {
        return this.apiRemove + '?index=' + params.index;
    }

    private reloadImage(force?: boolean): void {
        this.reloading = true;
        if (this.image) {
            this.image.src = this.svgURL(this.params, this.state, force);
        }
    }
    private imageReloaded(success: boolean): void {
        if (!this.reloading) return;

        if (success) {
            console.log("SVG loaded!");

        } else {
            console.warn("SVG load failed!");
        }
        this.setDisconnected(!success);
        this.reloading = false;
    }

    private compareRemote(remoteState: HttpgdState) {
        let needsReload: boolean = false;
        // server side changes
        if (!this.state.equals(remoteState)) {
            Object.assign(this.state, remoteState);
            this.params.index = -1; // fetch newest
            needsReload = true;
            this.notifyIndex();
        }
        // client side changes
        if (!this.params.equals(this.plotParams)) {
            Object.assign(this.plotParams, this.params);
            needsReload = true;
            this.notifyIndex();
        }
        // request new image
        if (needsReload) {
            this.reloadImage();
        }
    }

    private poll(): void {
        if (this.reloading) return; // don't poll while image is being loaded

        // fetch remote state
        fetch(this.apiState, {
            headers: this.apiHeaders
        })
            .then(res => res.json())
            .then((remoteState: HttpgdState) => {
                this.setDisconnected(false);
                if (this.reloading) return;
                this.compareRemote(remoteState);
            }).catch((e) => {
                console.warn(e);
                this.setDisconnected(true);
            });
    }

}