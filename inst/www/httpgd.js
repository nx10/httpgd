"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
class HttpgdApi {
    constructor(host, token) {
        this.httpHeaders = new Headers();
        this.http = 'http://' + host;
        this.ws = 'ws://' + host;
        this.httpSVG = this.http + '/svg';
        this.httpState = this.http + '/state';
        this.httpClear = this.http + '/clear';
        this.httpRemove = this.http + '/remove';
        this.httpPlots = this.http + '/plots';
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
    svg_index(index, width, height, c) {
        const url = this.svg_ext(width, height, c);
        url.searchParams.append('index', index.toString());
        return url;
    }
    svg_id(id, width, height, c) {
        const url = this.svg_ext(width, height, c);
        url.searchParams.append('id', id);
        return url;
    }
    svg_ext(width, height, c) {
        const url = new URL(this.httpSVG);
        if (width)
            url.searchParams.append('width', Math.round(width).toString());
        if (height)
            url.searchParams.append('height', Math.round(height).toString());
        if (this.useToken)
            url.searchParams.append('token', this.token);
        if (c)
            url.searchParams.append('c', c);
        return url;
    }
    remove_index(index) {
        const url = new URL(this.httpRemove);
        url.searchParams.append('index', index.toString());
        return url;
    }
    get_remove_index(index) {
        return __awaiter(this, void 0, void 0, function* () {
            const res = yield fetch(this.remove_index(index).href, {
                headers: this.httpHeaders
            });
            return res;
        });
    }
    remove_id(id) {
        const url = new URL(this.httpRemove);
        url.searchParams.append('id', id);
        return url;
    }
    get_remove_id(id) {
        return __awaiter(this, void 0, void 0, function* () {
            const res = yield fetch(this.remove_id(id).href, {
                headers: this.httpHeaders
            });
            return res;
        });
    }
    get_plots() {
        return __awaiter(this, void 0, void 0, function* () {
            const res = yield fetch(this.httpPlots, {
                headers: this.httpHeaders
            });
            return yield res.json();
        });
    }
    get_clear() {
        return __awaiter(this, void 0, void 0, function* () {
            const res = yield fetch(this.httpClear, {
                headers: this.httpHeaders
            });
            return res;
        });
    }
    get_state() {
        return __awaiter(this, void 0, void 0, function* () {
            const res = yield fetch(this.httpState, {
                headers: this.httpHeaders
            });
            return yield res.json();
        });
    }
    new_websocket() {
        return new WebSocket(this.ws);
    }
}
class HttpgdConnection {
    constructor(host, token, allowWebsockets) {
        this.mode = 0;
        this.pausePoll = false;
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
                this.pollHandle = setInterval(() => this.poll(), HttpgdConnection.INTERVAL_POLL);
                this.mode = targetMode;
                break;
            case 2:
                console.log("Start SLOWPOLL");
                this.clearWebsocket();
                this.clearPoll();
                this.pollHandle = setInterval(() => this.poll(), HttpgdConnection.INTERVAL_POLL_SLOW);
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
                this.socket = this.api.new_websocket();
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
        if (this.pausePoll)
            return;
        this.api.get_state().then((remoteState) => {
            this.setDisconnected(false);
            if (this.mode === 2)
                this.start(3);
            if (this.pausePoll)
                return;
            this.checkState(remoteState);
        }).catch((e) => {
            console.warn(e);
            this.setDisconnected(true);
        });
    }
    onWsMessage(message) {
        if (message.startsWith('{')) {
            const remoteState = JSON.parse(message);
            this.checkState(remoteState);
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
            (_a = this.connectionChanged) === null || _a === void 0 ? void 0 : _a.call(this, disconnected);
        }
    }
    checkState(remoteState) {
        var _a;
        if ((!this.lastState) ||
            (this.lastState.active !== remoteState.active) ||
            (this.lastState.hsize !== remoteState.hsize) ||
            (this.lastState.upid !== remoteState.upid)) {
            this.lastState = remoteState;
            (_a = this.remoteStateChanged) === null || _a === void 0 ? void 0 : _a.call(this, remoteState);
        }
    }
}
HttpgdConnection.INTERVAL_POLL = 500;
HttpgdConnection.INTERVAL_POLL_SLOW = 5000;
class HttpgdNavigator {
    constructor() {
        this.index = -1;
        this.width = 0;
        this.height = 0;
        this.last_id = "";
        this.last_width = 0;
        this.last_height = 0;
    }
    navigate(offset) {
        if (!this.data)
            return;
        this.index = (this.data.plots.length + this.index + offset) % this.data.plots.length;
    }
    jump(index) {
        if (!this.data)
            return;
        this.index = (this.data.plots.length + index) % this.data.plots.length;
    }
    jump_id(id) {
        if (!this.data)
            return;
        for (let i = 0; i < this.data.plots.length; i++) {
            if (id === this.data.plots[i].id) {
                this.index = i;
                break;
            }
        }
    }
    resize(width, height) {
        this.width = width;
        this.height = height;
    }
    next(api, c) {
        if (!this.data || this.data.plots.length == 0)
            return './plot-none.svg';
        if ((this.last_id !== this.data.plots[this.index].id) ||
            (Math.abs(this.last_width - this.width) > 0.1) ||
            (Math.abs(this.last_height - this.height) > 0.1))
            return api.svg_id(this.data.plots[this.index].id, this.width, this.height, c).href;
        return undefined;
    }
    update(data) {
        this.data = data;
        this.index = data.plots.length - 1;
    }
    id() {
        if (!this.data || this.data.plots.length == 0)
            return undefined;
        return this.data.plots[this.index].id;
    }
    indexStr() {
        if (!this.data)
            return '0/0';
        return Math.max(0, this.index + 1) + '/' + this.data.plots.length;
    }
}
class HttpgdViewer {
    constructor(host, token, allowWebsockets) {
        this.navi = new HttpgdNavigator();
        this.plotUpid = -1;
        this.scale = HttpgdViewer.SCALE_DEFAULT;
        this.deviceActive = true;
        this.image = undefined;
        this.sidebar = undefined;
        this.resizeBlocked = false;
        this.connection = new HttpgdConnection(host, token, allowWebsockets);
        this.connection.remoteStateChanged = (remoteState) => this.serverChanges(remoteState);
        this.connection.connectionChanged = (disconnected) => { var _a; return (_a = this.onDisconnectedChange) === null || _a === void 0 ? void 0 : _a.call(this, disconnected); };
    }
    init(image, sidebar) {
        var _a, _b;
        this.image = image;
        this.sidebar = sidebar;
        this.connection.open();
        this.checkResize();
        document.addEventListener('visibilitychange', () => {
            if (!document.hidden) {
                this.updateImage('v');
            }
        }, false);
        (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.navi.indexStr());
        (_b = this.onZoomStringChange) === null || _b === void 0 ? void 0 : _b.call(this, this.getZoomString());
        console.log('initial update plots');
        this.updatePlots(true);
    }
    updatePlots(scroll = false) {
        this.connection.api.get_plots().then(plots => {
            var _a;
            this.navi.update(plots);
            (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.navi.indexStr());
            this.updateSidebar(plots, scroll);
            this.updateImage();
        });
    }
    updateImage(c) {
        if (!this.image)
            return;
        const n = this.navi.next(this.connection.api, this.plotUpid + (c ? c : ''));
        if (n) {
            console.log('update image');
            this.image.src = n;
        }
    }
    updateSidebar(plots, scroll = false) {
        if (!this.sidebar)
            return;
        let idx = 0;
        while (idx < this.sidebar.children.length) {
            if (idx >= plots.plots.length || this.sidebar.children[idx].getAttribute('data-pid') !== plots.plots[idx].id) {
                this.sidebar.removeChild(this.sidebar.children[idx]);
            }
            else {
                idx++;
            }
        }
        for (; idx < plots.plots.length; ++idx) {
            const p = plots.plots[idx];
            const elem_card = document.createElement("div");
            elem_card.setAttribute('data-pid', p.id);
            const elem_x = document.createElement("a");
            elem_x.innerHTML = "&#10006;";
            elem_x.onclick = () => {
                this.connection.api.get_remove_id(p.id);
                this.updatePlots();
            };
            const elem_img = document.createElement("img");
            elem_card.classList.add("history-item");
            elem_img.setAttribute('src', this.connection.api.svg_id(p.id).href);
            elem_card.onclick = () => {
                var _a;
                this.navi.jump_id(p.id);
                (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.navi.indexStr());
                this.updateImage();
            };
            elem_card.appendChild(elem_img);
            elem_card.appendChild(elem_x);
            this.sidebar.appendChild(elem_card);
        }
        if (scroll) {
            this.sidebar.scrollTop = this.sidebar.scrollHeight;
        }
    }
    serverChanges(remoteState) {
        this.setDeviceActive(!remoteState.active);
        const lastUpid = this.plotUpid;
        this.plotUpid = remoteState.upid;
        if (lastUpid !== remoteState.upid)
            this.updatePlots(true);
    }
    setDeviceActive(active) {
        var _a;
        if (this.deviceActive !== active) {
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
        this.navi.navigate(-1);
        (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.navi.indexStr());
        this.updateImage();
    }
    navNext() {
        var _a;
        this.navi.navigate(1);
        (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.navi.indexStr());
        this.updateImage();
    }
    navNewest() {
        var _a;
        this.navi.jump(-1);
        (_a = this.onIndexStringChange) === null || _a === void 0 ? void 0 : _a.call(this, this.navi.indexStr());
        this.updateImage();
    }
    navClear() {
        this.connection.api.get_clear();
        this.updatePlots();
    }
    navRemove() {
        const id = this.navi.id();
        if (id) {
            this.connection.api.get_remove_id(id);
            this.updatePlots();
        }
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
    downloadPlotSVG(image) {
        if (!this.navi.id())
            return;
        fetch(image.src).then((response) => {
            return response.blob();
        }).then(blob => {
            HttpgdViewer.downloadURL(URL.createObjectURL(blob), 'plot_' + this.navi.id() + '.svg');
        });
    }
    static imageTempCanvas(image, fn) {
        const canvas = document.createElement('canvas');
        document.body.appendChild(canvas);
        const rect = image.getBoundingClientRect();
        canvas.width = rect.width;
        canvas.height = rect.height;
        const ctx = canvas.getContext('2d');
        if (!ctx)
            return;
        ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
        fn(canvas);
        document.body.removeChild(canvas);
    }
    downloadPlotPNG(image) {
        if (!this.navi.id())
            return;
        HttpgdViewer.imageTempCanvas(image, canvas => {
            const imgURI = canvas
                .toDataURL('image/png')
                .replace('image/png', 'image/octet-stream');
            HttpgdViewer.downloadURL(imgURI, 'plot_' + this.navi.id() + '.png');
        });
    }
    copyPlotPNG(image) {
        if (!this.navi.id())
            return;
        if (!navigator.clipboard)
            return;
        HttpgdViewer.imageTempCanvas(image, canvas => {
            canvas.toBlob(blob => {
                var _a, _b;
                if (blob)
                    (_b = (_a = navigator.clipboard).write) === null || _b === void 0 ? void 0 : _b.call(_a, [new ClipboardItem({ 'image/png': blob })]);
            });
        });
    }
    checkResize() {
        if (!this.image)
            return;
        const rect = this.image.getBoundingClientRect();
        this.navi.resize(rect.width * this.scale, rect.height * this.scale);
        this.updateImage();
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
HttpgdViewer.COOLDOWN_RESIZE = 200;
HttpgdViewer.SCALE_DEFAULT = 0.8;
HttpgdViewer.SCALE_STEP = HttpgdViewer.SCALE_DEFAULT / 12.0;
