import { HttpgdViewer } from '../viewer';
import { getById } from '../utils'

export class OverlayView {
    static readonly TEXT_CONNECTION_LOST: string = "Connection lost.";
    static readonly TEXT_DEVICE_INACTIVE: string = "Device inactive.";

    private viewer: HttpgdViewer;

    private overlayContainer: HTMLElement;
    private overlayText: HTMLElement;

    constructor(viewer: HttpgdViewer) {
        this.viewer = viewer;
        this.overlayContainer = getById("overlay");
        this.overlayText = getById("overlay-text");
    }

    public show(text: string): void {
        this.overlayText.innerText = text;
        this.overlayContainer.style.display = "inline";
    }

    public hide(): void {
        this.overlayContainer.style.display = "none";
    }
}