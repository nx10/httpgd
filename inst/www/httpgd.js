"use strict";
const CHECK_INTERVAL = 500;
const SCALE_DEFAULT = 0.8;
const SCALE_STEP = SCALE_DEFAULT / 12;
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
    }
    equals(other) {
        return this.upid === other.upid &&
            this.hsize === other.hsize;
    }
}
class HttpgdViewer {
    constructor(host, token, useWebsockets) {
        this.useWebsockets = false;
        this.apiHeaders = new Headers();
        this.pollIntervall = CHECK_INTERVAL;
        this.pollIntervallSlow = CHECK_INTERVAL * 10;
        this.scale = SCALE_DEFAULT;
        this.disconnected = false;
        this.canResize = true;
        this.resizeCooldown = CHECK_INTERVAL;
        this.state = new HttpgdState();
        this.params = new HttpgdParams();
        this.plotParams = new HttpgdParams();
        this.reloading = false;
        this.image = undefined;
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
        }
        else {
            this.useToken = false;
            this.token = '';
        }
        this.useWebsockets = (useWebsockets ? useWebsockets : false);
    }
    init(image) {
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
        }
        else {
            this.startPolling();
        }
        document.addEventListener('visibilitychange', () => {
            if (!document.hidden) {
                this.reloadImage(true);
            }
        }, false);
        this.notifyIndex();
        this.notifyZoom();
        this.notifyDisconnected();
    }
    onWsMessage(message) {
        if (message.startsWith('{')) {
            const remoteState = JSON.parse(message);
            this.compareRemote(remoteState);
        }
    }
    onWsClose() {
        console.log('Websocket closed');
        this.setDisconnected(true);
    }
    onWsOpen() {
        console.log('Websocket opened');
        this.setDisconnected(true);
    }
    startPolling() {
        if (this.pollHandle) {
            clearInterval(this.pollHandle);
        }
        this.pollHandle = setInterval(() => this.poll(), this.pollIntervall);
    }
    notifyZoom() {
        if (this.onZoomStringChange) {
            this.onZoomStringChange(this.getZoomString());
        }
    }
    notifyIndex() {
        if (this.onIndexStringChange) {
            this.onIndexStringChange(this.getIndexString());
        }
    }
    notifyDisconnected() {
        if (this.onDisconnectedChange) {
            this.onDisconnectedChange(this.disconnected);
        }
    }
    zoomIn() {
        if (this.scale - SCALE_STEP > 0.05) {
            this.scale -= SCALE_STEP;
        }
        this.notifyZoom();
        this.resize();
    }
    zoomOut() {
        this.scale += SCALE_STEP;
        this.notifyZoom();
        this.resize();
    }
    zoomReset() {
        this.scale = SCALE_DEFAULT;
        this.notifyZoom();
        this.resize();
    }
    getZoomString() {
        return Math.ceil(SCALE_DEFAULT / this.scale * 100) + '%';
    }
    navPrevious() {
        if (this.params.index > 0) {
            this.params.index -= 1;
        }
        else if (this.state.hsize >= 2) {
            this.params.index = this.state.hsize - (this.params.index == -1 ? 2 : 1);
        }
        this.notifyIndex();
        this.poll();
    }
    navNext() {
        if (this.params.index >= 0 && this.params.index < this.state.hsize - 1) {
            this.params.index += 1;
        }
        else {
            this.params.index = 0;
        }
        this.notifyIndex();
        this.poll();
    }
    navNewest() {
        this.params.index = -1;
        this.notifyIndex();
        this.poll();
    }
    getIndexString() {
        return (this.params.index === -1 ? this.state.hsize : (this.params.index + 1)) + '/' + this.state.hsize;
    }
    navClear() {
        fetch(this.apiClear, {
            headers: this.apiHeaders
        }).then(res => res.json())
            .then((remoteState) => {
            if (this.reloading)
                return;
            this.compareRemote(remoteState);
        });
    }
    navRemove() {
        fetch(this.removeURL(this.params), {
            headers: this.apiHeaders
        }).then(res => res.json())
            .then((remoteState) => {
            this.params.index = Math.max(-1, this.params.index - 1);
            this.compareRemote(remoteState);
        });
    }
    downloadURL(url, filename) {
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
        this.downloadURL(this.svgURL(this.plotParams, this.state), filename ? filename : 'plot.svg');
    }
    downloadPlotPNG() {
        const canvas = document.createElement('canvas');
        document.body.appendChild(canvas);
        canvas.width = this.plotParams.width;
        canvas.height = this.plotParams.height;
        const ctx = canvas.getContext('2d');
        if (!ctx)
            return;
        const img = new Image();
        img.crossOrigin = "anonymous";
        img.onload = () => {
            ctx.drawImage(img, 0, 0);
            var imgURI = canvas
                .toDataURL('image/png')
                .replace('image/png', 'image/octet-stream');
            this.downloadURL(imgURI, 'plot.png');
        };
        img.src = this.svgURL(this.plotParams, this.state);
        document.body.removeChild(canvas);
    }
    resize() {
        if (this.image) {
            const rect = this.image.getBoundingClientRect();
            this.params.width = rect.width * this.scale;
            this.params.height = rect.height * this.scale;
        }
        if (!this.canResize || this.reloading)
            return;
        this.canResize = false;
        this.poll();
        setTimeout(() => this.canResize = true, this.resizeCooldown);
    }
    setDisconnected(disconnected) {
        if (this.disconnected != disconnected) {
            this.disconnected = disconnected;
            clearInterval(this.pollHandle);
            this.pollHandle = setInterval(() => this.poll(), this.disconnected ? this.pollIntervallSlow : this.pollIntervall);
            this.notifyDisconnected();
        }
    }
    svgURL(params, state, force = false) {
        return this.apiSVG +
            '?width=' + Math.round(params.width) +
            '&height=' + Math.round(params.height) +
            '&index=' + params.index +
            (this.useToken ? ('&token=' + this.token) : '') +
            '&upid=' + state.upid + (force ? 'f' : '');
    }
    removeURL(params) {
        return this.apiRemove + '?index=' + params.index;
    }
    reloadImage(force) {
        this.reloading = true;
        if (this.image) {
            this.image.src = this.svgURL(this.params, this.state, force);
        }
    }
    imageReloaded(success) {
        if (!this.reloading)
            return;
        if (success) {
            console.log("SVG loaded!");
        }
        else {
            console.warn("SVG load failed!");
        }
        this.setDisconnected(!success);
        this.reloading = false;
    }
    compareRemote(remoteState) {
        let needsReload = false;
        if (!this.state.equals(remoteState)) {
            Object.assign(this.state, remoteState);
            this.params.index = -1;
            needsReload = true;
            this.notifyIndex();
        }
        if (!this.params.equals(this.plotParams)) {
            Object.assign(this.plotParams, this.params);
            needsReload = true;
            this.notifyIndex();
        }
        if (needsReload) {
            this.reloadImage();
        }
    }
    poll() {
        if (this.reloading)
            return;
        fetch(this.apiState, {
            headers: this.apiHeaders
        })
            .then(res => res.json())
            .then((remoteState) => {
            this.setDisconnected(false);
            if (this.reloading)
                return;
            this.compareRemote(remoteState);
        }).catch((e) => {
            console.warn(e);
            this.setDisconnected(true);
        });
    }
}
