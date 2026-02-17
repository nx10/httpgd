import type { IdResponse, PlotsResponse, PlotId } from "$lib/httpgd/types";
import { getPlotUrl } from "$lib/httpgd/api";

const SCALE_DEFAULT = 1.25;
const SCALE_STEP = SCALE_DEFAULT / 12.0;
const SCALE_MIN = 0.5;

class PlotsStore {
  plots = $state<IdResponse[]>([]);
  upid = $state("0");
  page = $state(1);
  zoom = $state(SCALE_DEFAULT);

  host = "";
  token?: string;

  get currentPlotId(): PlotId | null {
    if (this.plots.length === 0) return null;
    let p = this.page;
    const len = this.plots.length;
    while (p < 1) p += len;
    while (p > len) p -= len;
    return this.plots[p - 1].id;
  }

  get pageLabel(): string {
    return this.plots.length > 0 ? `${this.page}/${this.plots.length}` : "0/0";
  }

  get zoomLabel(): string {
    return Math.ceil((this.zoom / SCALE_DEFAULT) * 100) + "%";
  }

  configure(host: string, token?: string) {
    this.host = host;
    this.token = token;
  }

  updatePlots(data: PlotsResponse) {
    this.plots = data.plots;
    this.upid = data.state.upid.toString();
    this.page = data.plots.length;
  }

  getPlotImageUrl(
    width: number,
    height: number,
    plotId?: PlotId | null,
  ): string | null {
    const id = plotId ?? this.currentPlotId;
    if (!id) return null;
    return getPlotUrl(
      this.host,
      { id, width, height, zoom: this.zoom },
      this.token,
      this.upid,
    );
  }

  nextPage() {
    if (this.plots.length === 0) return;
    this.page = this.page >= this.plots.length ? 1 : this.page + 1;
  }

  prevPage() {
    if (this.plots.length === 0) return;
    this.page = this.page <= 1 ? this.plots.length : this.page - 1;
  }

  newestPage() {
    if (this.plots.length === 0) return;
    this.page = this.plots.length;
  }

  setPage(plotId: PlotId) {
    const idx = this.plots.findIndex((p) => p.id === plotId);
    if (idx >= 0) {
      this.page = idx + 1;
    }
  }

  zoomIn() {
    this.zoom += SCALE_STEP;
  }

  zoomOut() {
    if (this.zoom - SCALE_STEP > SCALE_MIN) {
      this.zoom -= SCALE_STEP;
    }
  }

  zoomReset() {
    this.zoom = SCALE_DEFAULT;
  }
}

export const plotsStore = new PlotsStore();
