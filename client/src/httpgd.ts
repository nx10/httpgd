import {
    HttpgdBackend, HttpgdStateResponse, HttpgdPlotsResponse, HttpgdRenderersResponse,
    HttpgdPlotRequest, HttpgdIdResponse, HttpgdRendererResponse, HttpgdRemoveRequest
} from './types';
import { fetch_clear, fetch_plots, fetch_remove, fetch_renderers, url_plot } from './api';
import { HttpgdConnection } from './connection';
import { StateChangeListener } from './utils';

interface HttpgdData {
    renderers: HttpgdRenderersResponse | null,
    plots: HttpgdPlotsResponse | null,
}

export class Httpgd {
    private backend: HttpgdBackend;
    private connection: HttpgdConnection;
    private data: HttpgdData;

    private plotsChanged: StateChangeListener<HttpgdPlotsResponse> = new StateChangeListener();
    private deviceActiveChanged: StateChangeListener<boolean> = new StateChangeListener();

    constructor(host: string, token?: string, allowWebsockets?: boolean) {
        this.data = { renderers: null, plots: null };
        this.backend = { host: host, token: token };
        this.connection = new HttpgdConnection(this.backend, allowWebsockets);
        this.connection.onRemoteChange((newState, oldState?) => this.remoteStateChanged(newState, oldState));
    }

    public connect(): Promise<void> {
        this.connection.open();
        return this.updateRenderers();
    }

    public onConnectionChange(fun: (newState: boolean, oldState?: boolean) => void): void {
        this.connection.onConnectionChange(fun);
    }

    private remoteStateChanged(newState: HttpgdStateResponse, oldState?: HttpgdStateResponse) {
        if (!oldState ||
            oldState.hsize !== newState.hsize ||
            oldState.upid !== newState.upid) {
            this.updatePlots();
        }
        if (!oldState ||
            oldState.active != newState.active) {
            this.deviceActiveChanged.notify(newState.active);
        }
    }

    private updateRenderers(): Promise<void> {
        return fetch_renderers(this.backend).then(res => {
            this.data.renderers = res;
        });
    }

    public updatePlots(): void {
        fetch_plots(this.backend).then(res => {
            this.data.plots = res;
            this.plotsChanged.notify(res);
        });
    }

    public getPlots(): HttpgdIdResponse[] {
        return this.data.plots.plots;
    }

    public getRenderers(): HttpgdRendererResponse[] {
        return this.data.renderers.renderers;
    }

    public getPlotURL(r: HttpgdPlotRequest): string {
        return url_plot(this.backend, r, true, this.data.plots.state.upid.toString());
    }

    public onPlotsChanged(fun: (newState: HttpgdPlotsResponse, oldState?: HttpgdPlotsResponse) => void): void {
        this.plotsChanged.subscribe(fun);
    }

    public onDeviceActiveChanged(fun: (newState: boolean, oldState?: boolean) => void): void {
        this.deviceActiveChanged.subscribe(fun);
    }

    public removePlot(r: HttpgdRemoveRequest): void {
        fetch_remove(this.backend, r);
    }

    public clearPlots(): void {
        fetch_clear(this.backend);
    }
}

