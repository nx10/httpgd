import { HttpgdBackend, HttpgdStateResponse } from './types';
import * as api from './api';
import { StateChangeListener } from './utils'

export const enum HttpgdConnectionMode {
    NONE,
    POLL,
    SLOWPOLL,
    WEBSOCKET
}

/**
 * Handles HTTP and WebSocket connection to httpgd server.
 * 
 * Will automatically fall back to HTTP polling if WebSocket is unavailable and
 * uses a slow polling mode to eventually reconnect when the server becomes 
 * completely unavailable.
 */
export class HttpgdConnection {
    private static readonly INTERVAL_POLL: number = 500;
    private static readonly INTERVAL_POLL_SLOW: number = 15000;

    public backend: HttpgdBackend;

    private mode: HttpgdConnectionMode = HttpgdConnectionMode.NONE;
    private allowWebsockets: boolean;

    private socket?: WebSocket;
    private pollHandle?: ReturnType<typeof setInterval>;

    private pausePoll: boolean = false;
    private disconnected: boolean = false;

    private lastState?: HttpgdStateResponse;

    private remoteStateChanged: StateChangeListener<HttpgdStateResponse> = new StateChangeListener();
    private connectionChanged: StateChangeListener<boolean> = new StateChangeListener();

    public constructor(backend: HttpgdBackend, allowWebsockets?: boolean) {
        this.backend = backend;
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
                //console.log("Start POLL");
                this.clearWebsocket();
                this.clearPoll();
                this.pollHandle = setInterval(() => this.poll(), HttpgdConnection.INTERVAL_POLL);
                this.mode = targetMode;
                break;
            case HttpgdConnectionMode.SLOWPOLL:
                //console.log("Start SLOWPOLL");
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
                //console.log("Start WEBSOCKET");
                this.clearPoll();
                this.clearWebsocket();

                this.socket = api.new_websocket(this.backend);
                this.socket.onmessage = (ev) => this.onWsMessage(ev.data);
                this.socket.onopen = () => this.onWsOpen();
                this.socket.onclose = () => this.onWsClose();
                this.socket.onerror = () => this.start(HttpgdConnectionMode.SLOWPOLL);
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
            this.socket.onclose = () => void(0);
            this.socket.close();
        }
    }

    private poll(): void {
        if (this.pausePoll) return;
        api.fetch_state(this.backend).catch((reason) => {
            throw reason;
        }).then((remoteState: HttpgdStateResponse) => {
            this.setDisconnected(false);
            if (this.mode === HttpgdConnectionMode.SLOWPOLL) this.start(HttpgdConnectionMode.WEBSOCKET); // reconnect
            if (this.pausePoll) return;
            this.checkState(remoteState);
        }).catch(() => {
            this.setDisconnected(true);
            if (this.mode !== HttpgdConnectionMode.SLOWPOLL) this.start(HttpgdConnectionMode.SLOWPOLL);
        });
    }

    private onWsMessage(message: string): void {
        if (message.startsWith('{')) {
            const remoteState = JSON.parse(message) as HttpgdStateResponse;
            this.checkState(remoteState);
        } else {
            console.log("Unknown WS message: " + message);
        }
    }
    private onWsClose(): void {
        //console.log('Websocket closed');
        this.setDisconnected(true);
    }
    private onWsOpen(): void {
        //console.log('Websocket opened');
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
            this.connectionChanged.notify(disconnected);
        }
    }

    private checkState(remoteState: HttpgdStateResponse): void {
        if (
            (!this.lastState) ||
            (this.lastState.active !== remoteState.active) ||
            (this.lastState.hsize !== remoteState.hsize) ||
            (this.lastState.upid !== remoteState.upid)
        ) {
            this.lastState = remoteState;
            this.remoteStateChanged.notify(remoteState);
        }
    }

    public onRemoteChange(fun: (newState: HttpgdStateResponse, oldState?: HttpgdStateResponse) => void): void {
        this.remoteStateChanged.subscribe(fun);
    }

    public onConnectionChange(fun: (newState: boolean, oldState?: boolean) => void): void {
        this.connectionChanged.subscribe(fun);
    }
}