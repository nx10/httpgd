export type PlotId = string;
export type RendererId = string;

export interface StateResponse {
  upid: number;
  hsize: number;
  active: boolean;
}

export interface IdResponse {
  id: PlotId;
}

export interface PlotsResponse {
  state: StateResponse;
  plots: IdResponse[];
}

export interface RendererResponse {
  id: RendererId;
  mime: string;
  ext: string;
  name: string;
  type: "plot" | "data";
  bin: boolean;
  descr: string;
}

export interface InfoResponse {
  id: string;
  version: string;
  unigd: string;
}

export interface PlotRequest {
  id?: PlotId;
  renderer?: RendererId;
  width?: number;
  height?: number;
  zoom?: number;
  download?: string;
}

export interface RemoveRequest {
  id: PlotId;
}
