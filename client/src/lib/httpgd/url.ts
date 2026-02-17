export interface ConnectionParams {
  host: string;
  token?: string;
  allowWebsockets: boolean;
  sidebarHidden: boolean;
}

export function parseConnectionParams(): ConnectionParams {
  const params = new URL(window.location.href).searchParams;
  return {
    host: params.get("hgd") || params.get("host") || window.location.host,
    token: params.get("token") || undefined,
    allowWebsockets: params.has("ws") ? params.get("ws") !== "0" : true,
    sidebarHidden: params.has("sidebar")
      ? params.get("sidebar") === "0"
      : false,
  };
}

export function httpUrl(host: string): string {
  const proto = window.location.protocol === "https:" ? "https" : "http";
  // If host already contains protocol, use as-is
  if (host.startsWith("http://") || host.startsWith("https://")) {
    return host;
  }
  return `${proto}://${host}`;
}

export function wsUrl(host: string): string {
  const proto = window.location.protocol === "https:" ? "wss" : "ws";
  if (host.startsWith("ws://") || host.startsWith("wss://")) {
    return host;
  }
  return `${proto}://${host}`;
}
