import type {
  StateResponse,
  PlotsResponse,
  RendererResponse,
  InfoResponse,
  PlotRequest,
  RemoveRequest,
} from "./types";
import { httpUrl } from "./url";

function headers(token?: string): HeadersInit {
  const h: Record<string, string> = {};
  if (token) {
    h["X-HTTPGD-TOKEN"] = token;
  }
  return h;
}

async function fetchJson<T>(url: string, token?: string): Promise<T> {
  const res = await fetch(url, { headers: headers(token) });
  if (!res.ok) throw new Error(`HTTP ${res.status}: ${res.statusText}`);
  return res.json();
}

export async function fetchState(
  host: string,
  token?: string,
): Promise<StateResponse> {
  return fetchJson<StateResponse>(`${httpUrl(host)}/state`, token);
}

export async function fetchPlots(
  host: string,
  token?: string,
): Promise<PlotsResponse> {
  return fetchJson<PlotsResponse>(`${httpUrl(host)}/plots`, token);
}

export async function fetchRenderers(
  host: string,
  token?: string,
): Promise<RendererResponse[]> {
  const data = await fetchJson<{ renderers: RendererResponse[] }>(
    `${httpUrl(host)}/renderers`,
    token,
  );
  return data.renderers;
}

export async function fetchInfo(
  host: string,
  token?: string,
): Promise<InfoResponse> {
  return fetchJson<InfoResponse>(`${httpUrl(host)}/info`, token);
}

export async function removePlot(
  host: string,
  req: RemoveRequest,
  token?: string,
): Promise<void> {
  const url = new URL(`${httpUrl(host)}/remove`);
  url.searchParams.set("id", req.id);
  await fetch(url.toString(), { headers: headers(token) });
}

export async function clearPlots(host: string, token?: string): Promise<void> {
  await fetch(`${httpUrl(host)}/clear`, { headers: headers(token) });
}

export function getPlotUrl(
  host: string,
  req: PlotRequest,
  token?: string,
  cacheStr?: string,
): string {
  const url = new URL(`${httpUrl(host)}/plot`);
  if (req.id) url.searchParams.set("id", req.id);
  if (req.renderer) url.searchParams.set("renderer", req.renderer);
  if (req.width != null)
    url.searchParams.set("width", Math.round(req.width).toString());
  if (req.height != null)
    url.searchParams.set("height", Math.round(req.height).toString());
  if (req.zoom != null) url.searchParams.set("zoom", req.zoom.toString());
  if (req.download) url.searchParams.set("download", req.download);
  if (token) url.searchParams.set("token", token);
  if (cacheStr) url.searchParams.set("c", cacheStr);
  return url.toString();
}
