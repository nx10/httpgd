/* eslint-disable @typescript-eslint/no-inferrable-types */
import { HttpgdViewer } from '../viewer';
import { HttpgdPlotsResponse } from '../types';
import { getById, downloadImgSVG, downloadImgPNG, copyImgSVGasPNG } from '../utils'
import { ASSET_PLOT_NONE } from '../resources';
import { ToolbarView } from './toolbarView';
import { SidebarView } from './sidebarView';

export class PlotView {
    static readonly COOLDOWN_RESIZE: number = 200;
    static readonly SCALE_DEFAULT: number = 1.25;
    static readonly SCALE_STEP: number = PlotView.SCALE_DEFAULT / 12.0;
    static readonly SCALE_MIN: number = 0.5;

    private viewer: HttpgdViewer;
    public toolbar: ToolbarView;
    public sidebar: SidebarView;

    private image: HTMLImageElement;

    private resizeBlocked: boolean = false;
    private scale: number = PlotView.SCALE_DEFAULT; // zoom level
    private page: number = 1;
    private plots?: HttpgdPlotsResponse;

    constructor(viewer: HttpgdViewer, sidebarHidden?: boolean) {
        this.viewer = viewer;
        this.toolbar = new ToolbarView(viewer);
        this.sidebar = new SidebarView(viewer, sidebarHidden);
        this.image = getById("drawing");
        this.image.src = ASSET_PLOT_NONE;
        window.addEventListener("resize", () => this.resize());

        // TODO
        /*
        // Force reload on visibility change
        // Firefox otherwise shows a blank screen on tab change 
        document.addEventListener('visibilitychange', () => {
            if (!document.hidden) {
                this.updateImage('v');
            }
        }, false);*/
    }

    public updatePlots(newState: HttpgdPlotsResponse): void {
        this.plots = newState;
        this.page = newState.plots.length;
        this.updatePageLabel();
        this.sidebar.update(newState);
    }

    public getCurrentPlotId(): string | null {
        if (!this.plots) return null;
        while (this.page < 1) {
            this.page += this.plots.plots.length;
        }
        while (this.page > this.plots.plots.length) {
            this.page -= this.plots.plots.length;
        }
        return this.plots.plots[this.page - 1].id;
    }

    public update(): void {
        if (!this.plots || this.plots.plots.length == 0) {
            this.image.src = ASSET_PLOT_NONE;
            return;
        }

        const plotId = this.getCurrentPlotId();
        const rect = this.image.getBoundingClientRect();
        const url = this.viewer.httpgd.getPlotURL({
            id: plotId,
            width: rect.width,
            height: rect.height,
            zoom: this.scale,
        })
        if (url) {
            this.image.src = url;
        }

        this.updatePageLabel();
        this.sidebar.setSelected(plotId);
    }

    private resize() {
        if (this.resizeBlocked) return;
        this.resizeBlocked = true;
        setTimeout(() => {
            this.update();
            this.resizeBlocked = false;
        }, PlotView.COOLDOWN_RESIZE);
    }

    // Zooming
    public zoomOut(): void {
        if (this.scale - PlotView.SCALE_STEP > PlotView.SCALE_MIN) {
            this.scale -= PlotView.SCALE_STEP;
        }
        this.updateZoomLabel();
        this.resize();
    }
    public zoomIn(): void {
        this.scale += PlotView.SCALE_STEP;
        this.updateZoomLabel();
        this.resize();
    }
    public zoomReset(): void {
        this.scale = PlotView.SCALE_DEFAULT;
        this.updateZoomLabel();
        this.resize();
    }
    private getZoomString(): string {
        return Math.ceil(this.scale / PlotView.SCALE_DEFAULT * 100) + '%';
    }
    private updateZoomLabel() {
        this.toolbar.setZoomLabelText(this.getZoomString());
    }

    // Pagenation
    public nextPage(): void {
        this.page++;
        this.update();
    }
    public prevPage(): void {
        this.page--;
        this.update();
    }
    public newestPage(): void {
        if (!this.plots) return;
        if (this.page == this.plots.plots.length) return;
        this.page = this.plots.plots.length;
        this.update();
    }
    public setPage(plotId: string): void {
        if (!this.plots) return;
        for (let i = 0; i < this.plots.plots.length; ++i) {
            if (this.plots.plots[i].id === plotId) {
                this.page = i + 1;
                this.update();
                return;
            }
        }
    }
    private getPageString(): string {
        return this.plots ? (this.page + "/" + this.plots.plots.length) : "0/0";
    }
    private updatePageLabel() {
        this.toolbar.setPageLabelText(this.getPageString());
    }

    public downloadSVG(): void {
        downloadImgSVG(this.image, 'plot.svg');
    }

    public downloadPNG(): void {
        // todo: download server side rendered PNG if available
        downloadImgPNG(this.image, 'plot.png');
    }

    public copyPNG(): void {
        // todo: copy server side rendered PNG if available
        copyImgSVGasPNG(this.image);
    }
    public removePlot(): void {
        this.viewer.httpgd.removePlot({ id: this.getCurrentPlotId() });
    }
    public clearPlots(): void {
        this.viewer.httpgd.clearPlots();
    }
}