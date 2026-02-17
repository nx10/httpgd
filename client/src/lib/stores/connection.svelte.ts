import type { InfoResponse, RendererResponse } from "$lib/httpgd/types";

class ConnectionStore {
  connected = $state(false);
  deviceActive = $state(true);
  info = $state<InfoResponse | undefined>(undefined);
  renderers = $state<RendererResponse[]>([]);

  setConnected(value: boolean) {
    this.connected = value;
  }

  setDeviceActive(value: boolean) {
    this.deviceActive = value;
  }

  setInfo(value: InfoResponse) {
    this.info = value;
  }

  setRenderers(value: RendererResponse[]) {
    this.renderers = value;
  }
}

export const connectionStore = new ConnectionStore();
