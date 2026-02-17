import type { StateResponse } from "./types";
import { wsUrl } from "./url";

export type WsMessageHandler = (state: StateResponse) => void;
export type WsStatusHandler = (connected: boolean) => void;

export class WebSocketManager {
  private ws: WebSocket | null = null;
  private host: string;
  private onMessage: WsMessageHandler;
  private onStatus: WsStatusHandler;
  private reconnectTimer: ReturnType<typeof setTimeout> | null = null;
  private closed = false;

  constructor(
    host: string,
    onMessage: WsMessageHandler,
    onStatus: WsStatusHandler,
  ) {
    this.host = host;
    this.onMessage = onMessage;
    this.onStatus = onStatus;
  }

  connect(): void {
    this.closed = false;
    this.clearReconnect();

    try {
      this.ws = new WebSocket(wsUrl(this.host));
    } catch {
      this.onStatus(false);
      this.scheduleReconnect();
      return;
    }

    this.ws.onopen = () => {
      this.onStatus(true);
    };

    this.ws.onmessage = (event) => {
      try {
        const data: StateResponse = JSON.parse(event.data);
        this.onMessage(data);
      } catch {
        // ignore malformed messages
      }
    };

    this.ws.onclose = () => {
      this.onStatus(false);
      if (!this.closed) {
        this.scheduleReconnect();
      }
    };

    this.ws.onerror = () => {
      this.ws?.close();
    };
  }

  disconnect(): void {
    this.closed = true;
    this.clearReconnect();
    if (this.ws) {
      this.ws.onclose = null;
      this.ws.onerror = null;
      this.ws.close();
      this.ws = null;
    }
  }

  private scheduleReconnect(): void {
    if (this.closed) return;
    this.clearReconnect();
    this.reconnectTimer = setTimeout(() => this.connect(), 2000);
  }

  private clearReconnect(): void {
    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
  }
}
