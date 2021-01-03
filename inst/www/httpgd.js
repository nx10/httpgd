"use strict";
class ImageRequestHelper {
    constructor(image, onSuccess, onError) {
        this._image = image;
        const _this = this;
        this._success = function success(ev) {
            onSuccess === null || onSuccess === void 0 ? void 0 : onSuccess(image);
            _this.cleanup();
        };
        this._error = function error(ev) {
            onError === null || onError === void 0 ? void 0 : onError(image);
            _this.cleanup();
        };
        image.addEventListener('load', this._success);
        image.addEventListener('error', this._error);
    }
    cleanup() {
        this._image.removeEventListener('load', this._success);
        this._image.removeEventListener('error', this._error);
    }
}
class HttpgdParams {
    constructor() {
        this.index = -1;
        this.width = 0;
        this.height = 0;
    }
    equals(other) {
        return this.index === other.index &&
            Math.abs(this.width - other.width) < 0.1 &&
            Math.abs(this.height - other.height) < 0.1;
    }
}
class HttpgdState {
    constructor() {
        this.upid = -1;
        this.hsize = 0;
        this.active = true;
    }
    equals(other) {
        return this.upid === other.upid &&
            this.hsize === other.hsize &&
            this.active === other.active;
    }
}
class HttpgdApi {
    constructor(host, token) {
        this.httpHeaders = new Headers();
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
        }
        else {
            this.useToken = false;
            this.token = '';
        }
    }
    svgURL(width, height, index, upid, force = false) {
        return this.httpSVG +
            '?width=' + Math.round(width) +
            '&height=' + Math.round(height) +
            '&index=' + index +
            (this.useToken ? ('&token=' + this.token) : '') +
            '&upid=' + upid + (force ? 'f' : '');
    }
    removeURL(index) {
        return this.httpRemove + '?index=' + index;
    }
}
class HttpgdConnectionManager {
    constructor(host, token, allowWebsockets) {
        this.mode = 0;
        this.imageReloading = false;
        this.disconnected = true;
        this.api = new HttpgdApi(host, token);
        this.allowWebsockets = allowWebsockets ? allowWebsockets : false;
    }
    open() {
        if (this.mode != 0)
            return;
        this.start(3);
    }
    close() {
        if (this.mode == 0)
            return;
        this.start(0);
    }
    start(targetMode) {
        if (this.mode == targetMode)
            return;
        switch (targetMode) {
            case 1:
                console.log("Start POLL");
                this.clearWebsocket();
                this.clearPoll();
                this.pollHandle = setInterval(() => this.poll(), HttpgdConnectionManager.INTERVAL_POLL);
                this.mode = targetMode;
                break;
            case 2:
                console.log("Start SLOWPOLL");
                this.clearWebsocket();
                this.clearPoll();
                this.pollHandle = setInterval(() => this.poll(), HttpgdConnectionManager.INTERVAL_POLL_SLOW);
                this.mode = targetMode;
                break;
            case 3:
                if (!this.allowWebsockets) {
                    this.start(1);
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
                this.poll();
                break;
            case 0:
                this.clearWebsocket();
                this.clearPoll();
                this.mode = targetMode;
                break;
            default:
                break;
        }
    }
    clearPoll() {
        if (this.pollHandle) {
            clearInterval(this.pollHandle);
        }
    }
    clearWebsocket() {
        if (this.socket) {
            this.socket.onclose = () => { };
            this.socket.close();
        }
    }
    poll() {
        if (this.imageReloading)
            return;
        fetch(this.api.httpState, {
            headers: this.api.httpHeaders
        })
            .then(res => res.json())
            .then((remoteState) => {
            var _a;
            this.setDisconnected(false);
            if (this.imageReloading)
                return;
            (_a = this.stateCallback) === null || _a === void 0 ? void 0 : _a.call(this, remoteState);
        }).catch((e) => {
            console.warn(e);
            this.setDisconnected(true);
        });
    }
    onWsMessage(message) {
        var _a;
        if (message.startsWith('{')) {
            const remoteState = JSON.parse(message);
            (_a = this.stateCallback) === null || _a === void 0 ? void 0 : _a.call(this, remoteState);
        }
        else {
            console.log("Unknown WS message: " + message);
        }
    }
    onWsClose() {
        console.log('Websocket closed');
        this.setDisconnected(true);
    }
    onWsOpen() {
        console.log('Websocket opened');
        this.setDisconnected(false);
    }
    setDisconnected(disconnected) {
        var _a;
        if (this.disconnected != disconnected) {
            this.disconnected = disconnected;
            if (this.disconnected) {
                this.start(2);
            }
            else {
                this.start(3);
            }
            (_a = this.connectionCallback) === null || _a === void 0 ? void 0 : _a.call(this, disconnected);
        }
    }
    loadImage(width, height, index, upid, force, image) {
        if (!image)
            return;
        new ImageRequestHelper(image, (image) => {
            this.imageReloading = false;
        }, (image) => {
            console.warn("SVG load failed!");
            this.imageReloading = false;
        });
        image.src = this.api.svgURL(width, height, index, upid, force);
    }
    clearPlots() {
        fetch(this.api.httpClear, {
            headers: this.api.httpHeaders
        }).then(res => res.json())
            .then((remoteState) => {
            var _a;
            (_a = this.stateCallback) === null || _a === void 0 ? void 0 : _a.call(this, remoteState);
        });
    }
    removePlot(index) {
        fetch(this.api.removeURL(index), {
            headers: this.api.httpHeaders
        }).then(res => res.json())
            .then((remoteState) => {
            var _a;
            (_a = this.stateCallbackRemove) === null || _a === void 0 ? void 0 : _a.call(this, remoteState);
        });
    }
    svgURL(width, height, index, upid) {
        return this.api.svgURL(width, height, index, upid);
    }
}
HttpgdConnectionManager.INTERVAL_POLL = 500;
HttpgdConnectionManager.INTERVAL_POLL_SLOW = 5000;
class HttpgdViewer {
    constructor(host, token, allowWebsockets) {
        this.state = new HttpgdState();
        this.params = new HttpgdParams();
        this.plotParams = new HttpgdParams();
        this.scale = HttpgdViewer.SCALE_DEFAULT;
        this.deviceActive = true;
        this.image = undefined;
        this.resizeBlocked = false;
        this.connection = new HttpgdConnectionManager(host, token, allowWebsockets);
        this.connection.stateCallback = (remoteState) => this.serverChanges(remoteState);
        this.connection.stateCallbackRemove = (remoteState) => {
            this.params.index = Math.max(-1, this.params.index - 1);
            this.serverChanges(remoteState);
        };
        this.connection.connectionCallback = (disconnected) => { var _a; return (_a = this.onDisconnectedChange) === null || _a === void 0 ? void 0 : _a.call(this, disconnected); };
    }
    init(image) {
        var _a, _b;
        this.image = image;
        this.connection.open();
        this.checkResize();
        document.addEventListener('visibilitychange', () => {
            if (!document.hidden) {
                this.loadImage(true);
            }
        }, false);
        (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.getIndexString());
        (_b = this.onZoomStringChange) === null || _b === void 0 ? void 0 : _b.call(this, this.getZoomString());
    }
    loadImage(force) {
        this.connection.loadImage(this.params.width, this.params.height, this.params.index, this.state.upid, force, this.image);
    }
    serverChanges(remoteState) {
        var _a;
        this.setDeviceActive(!remoteState.active);
        let needsReload = false;
        if (this.diffServer(remoteState)) {
            Object.assign(this.state, remoteState);
            this.params.index = -1;
            needsReload = true;
        }
        if (this.diffClient()) {
            Object.assign(this.plotParams, this.params);
            needsReload = true;
        }
        if (needsReload) {
            (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.getIndexString());
            this.loadImage();
        }
    }
    clientChanges() {
        var _a;
        if (this.diffClient()) {
            Object.assign(this.plotParams, this.params);
            (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.getIndexString());
            this.loadImage();
        }
    }
    diffServer(remoteState) {
        return !this.state.equals(remoteState);
    }
    diffClient() {
        return !this.params.equals(this.plotParams);
    }
    setDeviceActive(active) {
        var _a;
        if (this.deviceActive != active) {
            this.deviceActive = active;
            (_a = this.onDeviceActiveChange) === null || _a === void 0 ? void 0 : _a.call(this, active);
        }
    }
    zoomIn() {
        var _a;
        if (this.scale - HttpgdViewer.SCALE_STEP > 0.05) {
            this.scale -= HttpgdViewer.SCALE_STEP;
        }
        (_a = this.onZoomStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.getZoomString());
        this.checkResize();
    }
    zoomOut() {
        var _a;
        this.scale += HttpgdViewer.SCALE_STEP;
        (_a = this.onZoomStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.getZoomString());
        this.checkResize();
    }
    zoomReset() {
        var _a;
        this.scale = HttpgdViewer.SCALE_DEFAULT;
        (_a = this.onZoomStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.getZoomString());
        this.checkResize();
    }
    getZoomString() {
        return Math.ceil(HttpgdViewer.SCALE_DEFAULT / this.scale * 100) + '%';
    }
    navPrevious() {
        var _a;
        if (this.params.index > 0) {
            this.params.index -= 1;
        }
        else if (this.state.hsize >= 2) {
            this.params.index = this.state.hsize - (this.params.index == -1 ? 2 : 1);
        }
        (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.getIndexString());
        this.clientChanges();
    }
    navNext() {
        var _a;
        if (this.params.index >= 0 && this.params.index < this.state.hsize - 1) {
            this.params.index += 1;
        }
        else {
            this.params.index = 0;
        }
        (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.getIndexString());
        this.clientChanges();
    }
    navNewest() {
        var _a;
        this.params.index = -1;
        (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.getIndexString());
        this.clientChanges();
    }
    getIndexString() {
        return (this.params.index === -1 ? this.state.hsize : (this.params.index + 1)) + '/' + this.state.hsize;
    }
    navClear() {
        this.connection.clearPlots();
    }
    navRemove() {
        this.connection.removePlot(this.params.index);
    }
    svgURL() {
        return this.connection.svgURL(this.plotParams.width, this.plotParams.height, this.plotParams.index, this.state.upid);
    }
    static downloadURL(url, filename) {
        const dl = document.createElement('a');
        dl.href = url;
        if (filename) {
            dl.download = filename;
        }
        document.body.appendChild(dl);
        dl.click();
        document.body.removeChild(dl);
    }
    downloadPlot(filename) {
        HttpgdViewer.downloadURL(this.svgURL(), filename ? filename : 'plot.svg');
    }
    downloadPlotPNG() {
        const canvas = document.createElement('canvas');
        document.body.appendChild(canvas);
        canvas.width = this.plotParams.width / this.scale;
        canvas.height = this.plotParams.height / this.scale;
        const ctx = canvas.getContext('2d');
        if (!ctx)
            return;
        const img = new Image();
        img.crossOrigin = "anonymous";
        img.onload = () => {
            ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
            const imgURI = canvas
                .toDataURL('image/png')
                .replace('image/png', 'image/octet-stream');
            HttpgdViewer.downloadURL(imgURI, 'plot.png');
        };
        img.src = this.svgURL();
        document.body.removeChild(canvas);
    }
    copyPlotPNG() {
        if (!navigator.clipboard)
            return;
        const canvas = document.createElement('canvas');
        document.body.appendChild(canvas);
        canvas.width = this.plotParams.width / this.scale;
        canvas.height = this.plotParams.height / this.scale;
        const ctx = canvas.getContext('2d');
        if (!ctx)
            return;
        const img = new Image();
        img.crossOrigin = 'anonymous';
        img.onload = () => {
            ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
            canvas.toBlob(blob => { var _a, _b; if (blob)
                (_b = (_a = navigator.clipboard).write) === null || _b === void 0 ? void 0 : _b.call(_a, [new ClipboardItem({ 'image/png': blob })]); });
        };
        img.src = this.svgURL();
        document.body.removeChild(canvas);
    }
    checkResize() {
        if (!this.image)
            return;
        const rect = this.image.getBoundingClientRect();
        this.params.width = rect.width * this.scale;
        this.params.height = rect.height * this.scale;
        this.clientChanges();
    }
    resize() {
        if (this.resizeBlocked)
            return;
        this.resizeBlocked = true;
        setTimeout(() => {
            this.checkResize();
            this.resizeBlocked = false;
        }, HttpgdViewer.COOLDOWN_RESIZE);
    }
}
HttpgdViewer.COOLDOWN_RESIZE = 500;
HttpgdViewer.SCALE_DEFAULT = 0.8;
HttpgdViewer.SCALE_STEP = HttpgdViewer.SCALE_DEFAULT / 12.0;
