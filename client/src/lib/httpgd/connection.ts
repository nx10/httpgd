import type {
  StateResponse,
  PlotsResponse,
  RendererResponse,
  InfoResponse,
} from "./types";
import { fetchState, fetchPlots, fetchRenderers, fetchInfo } from "./api";
import { WebSocketManager } from "./websocket";

const POLL_INTERVAL = 500;

enum ConnectionMode {
  NONE,
  POLL,
  WEBSOCKET,
}

export interface ConnectionCallbacks {
  onPlotsChanged: (plots: PlotsResponse) => void;
  onConnectionChanged: (connected: boolean) => void;
  onDeviceActiveChanged: (active: boolean) => void;
  onRenderersChanged: (renderers: RendererResponse[]) => void;
}

export class Connection {
  private host: string;
  private token?: string;
  private allowWebsockets: boolean;
  private callbacks: ConnectionCallbacks;

  private mode = ConnectionMode.NONE;
  private lastState?: StateResponse;
  private pollTimer: ReturnType<typeof setInterval> | null = null;
  private wsManager: WebSocketManager | null = null;

  info?: InfoResponse;
  renderers: RendererResponse[] = [];
  plots?: PlotsResponse;

  constructor(
    host: string,
    token: string | undefined,
    allowWebsockets: boolean,
    callbacks: ConnectionCallbacks,
  ) {
    this.host = host;
    this.token = token;
    this.allowWebsockets = allowWebsockets;
    this.callbacks = callbacks;
  }

  async connect(): Promise<void> {
    try {
      this.info = await fetchInfo(this.host, this.token);
      await this.updateRenderers();
      await this.updatePlots();
    } catch {
      // will retry via polling
    }

    if (this.allowWebsockets) {
      this.startWebSocket();
    } else {
      this.startPolling();
    }
  }

  disconnect(): void {
    this.stopPolling();
    this.wsManager?.disconnect();
    this.wsManager = null;
    this.mode = ConnectionMode.NONE;
  }

  private async updateRenderers(): Promise<void> {
    try {
      this.renderers = await fetchRenderers(this.host, this.token);
      this.callbacks.onRenderersChanged(this.renderers);
    } catch {
      // ignore fetch errors
    }
  }

  async updatePlots(): Promise<void> {
    try {
      const plots = await fetchPlots(this.host, this.token);
      this.plots = plots;
      this.callbacks.onPlotsChanged(plots);
    } catch {
      // ignore fetch errors
    }
  }

  getUpid(): string {
    return this.plots?.state.upid.toString() ?? "0";
  }

  async reconnect(): Promise<void> {
    this.disconnect();
    await this.connect();
  }

  private startWebSocket(): void {
    this.stopPolling();
    this.mode = ConnectionMode.WEBSOCKET;

    this.wsManager = new WebSocketManager(
      this.host,
      (state) => this.handleRemoteState(state),
      (connected) => {
        if (connected) {
          this.callbacks.onConnectionChanged(true);
          // Refresh on reconnect
          this.updateRenderers();
          this.updatePlots();
        } else {
          this.callbacks.onConnectionChanged(false);
          this.wsManager?.disconnect();
          this.wsManager = null;
          this.stopPolling();
          this.mode = ConnectionMode.NONE;
        }
      },
    );
    this.wsManager.connect();
  }

  private startPolling(): void {
    this.stopPolling();
    this.mode = ConnectionMode.POLL;
    this.pollTimer = setInterval(() => this.poll(), POLL_INTERVAL);
  }

  private stopPolling(): void {
    if (this.pollTimer) {
      clearInterval(this.pollTimer);
      this.pollTimer = null;
    }
  }

  private async poll(): Promise<void> {
    try {
      const state = await fetchState(this.host, this.token);
      this.handleRemoteState(state);
    } catch {
      if (this.mode === ConnectionMode.POLL) {
        this.callbacks.onConnectionChanged(false);
        this.stopPolling();
        this.mode = ConnectionMode.NONE;
      }
    }
  }

  private handleRemoteState(state: StateResponse): void {
    const old = this.lastState;
    this.lastState = state;

    if (!old || old.active !== state.active) {
      this.callbacks.onDeviceActiveChanged(state.active);
    }

    if (!old || old.upid !== state.upid || old.hsize !== state.hsize) {
      this.updatePlots();
    }
  }
}
