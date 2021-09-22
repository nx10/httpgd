import { HttpgdViewer } from '../viewer';
import { HttpgdRendererResponse } from 'httpgd/lib/types';
import { downloadURL, getById, validNumberInput, setCssClass, strcmp } from '../utils'
import { ASSET_PLOT_NONE } from '../resources';

export class ExportView {
    static readonly MIN_PREVIEW_SIZE: number = 1;
    static readonly MAX_PREVIEW_SIZE: number = 10000;
    static readonly MIN_PREVIEW_ZOOM: number = 0.01;
    static readonly MAX_PREVIEW_ZOOM: number = 10000;

    private viewer: HttpgdViewer;

    private renderers: HttpgdRendererResponse[];

    private elemModal: HTMLElement;
    private imgPreview: HTMLImageElement;
    private inputWidth: HTMLInputElement;
    private inputHeight: HTMLInputElement;
    private inputZoom: HTMLInputElement;
    private btnOpen: HTMLButtonElement;
    private btnDownload: HTMLButtonElement;
    private selectFormat: HTMLSelectElement;
    private btnClose: HTMLElement;

    constructor(viewer: HttpgdViewer) {
        this.viewer = viewer;

        this.elemModal = getById("exp-modal");
        this.imgPreview = getById("exp-image");
        this.inputWidth = getById("ie-width");
        this.inputHeight = getById("ie-height");
        this.inputZoom = getById("ie-scale");
        this.btnOpen = getById("ie-btn-open");
        this.btnDownload = getById("ie-btn-download");
        this.selectFormat = getById("ie-format");
        this.btnClose = getById("exp-modal-close");

        this.btnClose.onclick = () => this.hide();
        window.onmousedown = (event: MouseEvent) => {
            if (event.target == this.elemModal) {
                this.hide();
            }
        }

        this.inputWidth.addEventListener('input', () => this.update());
        this.inputHeight.addEventListener('input', () => this.update());
        this.inputZoom.addEventListener('input', () => this.update());
        this.btnDownload.onclick = () => this.clickDownload();
        this.btnOpen.onclick = () => this.clickOpen();
    }

    public initRenderers(): void {
        this.renderers = this.viewer.httpgd.getRenderers();
        this.renderers.sort((a, b) => strcmp(a.name, b.name)).forEach((r) => {
            const o = document.createElement("option");
            o.value = r.id;
            o.text = r.name + " (*"+ r.ext + ")";
            this.selectFormat.add(o);
        });
        this.selectFormat.value = "svg";
    }

    private hide() {
        this.elemModal.style.display = "none";
    }

    public isVisible(): boolean {
        return this.elemModal.style.display &&
            this.elemModal.style.display !== "none";
    }

    private validWidth(): boolean {
        return validNumberInput(this.inputWidth, ExportView.MIN_PREVIEW_SIZE, ExportView.MAX_PREVIEW_SIZE);
    }

    private getWidth(): number {
        return Math.min(parseInt(this.inputWidth.value), ExportView.MAX_PREVIEW_SIZE);
    }

    private validHeight(): boolean {
        return validNumberInput(this.inputHeight, ExportView.MIN_PREVIEW_SIZE, ExportView.MAX_PREVIEW_SIZE);
    }

    private getHeight(): number {
        return Math.min(parseInt(this.inputHeight.value), ExportView.MAX_PREVIEW_SIZE)
    }

    private validZoom(): boolean {
        return validNumberInput(this.inputZoom, ExportView.MIN_PREVIEW_ZOOM, ExportView.MAX_PREVIEW_ZOOM);
    }

    private getZoom(): number {
        return Math.max(parseInt(this.inputZoom.value) / 100, ExportView.MIN_PREVIEW_ZOOM);
    }

    private getRenderer(): HttpgdRendererResponse {
        return this.viewer.httpgd.getRenderers().find((r) => r.id == this.selectFormat.value);
    }

    private clickDownload() {
        const plotId = this.viewer.plotView.getCurrentPlotId();
        if (!plotId) return;
        const renderer = this.getRenderer();
        const url = this.viewer.httpgd.getPlotURL({
            width: this.getWidth(),
            height: this.getHeight(),
            zoom: this.getZoom(),
            id: plotId,
            renderer: renderer.id,
            download: "plot_" + plotId + renderer.ext,
        });
        downloadURL(url);
    }

    private clickOpen() {
        const plotId = this.viewer.plotView.getCurrentPlotId();
        if (!plotId) return;
        const renderer = this.getRenderer();
        const url = this.viewer.httpgd.getPlotURL({
            width: this.getWidth(),
            height: this.getHeight(),
            zoom: this.getZoom(),
            id: plotId,
            renderer: renderer.id,
        });
        downloadURL(url, null, true);
    }

    private update() {
        if (setCssClass(this.inputWidth, !this.validWidth(), "invalid-input") ||
            setCssClass(this.inputHeight, !this.validHeight(), "invalid-input") ||
            setCssClass(this.inputZoom, !this.validZoom(), "invalid-input")) 
            return;

        const plotId = this.viewer.plotView.getCurrentPlotId();
        if (!plotId) {
            this.imgPreview.src = ASSET_PLOT_NONE;
            return;
        }
        const url = this.viewer.httpgd.getPlotURL({
            width: this.getWidth(),
            height: this.getHeight(),
            zoom: this.getZoom(),
            id: plotId,
        });
        this.imgPreview.src = url;
    }

    public show(): void {
        this.elemModal.style.display = "block";
        this.update();
    }
}