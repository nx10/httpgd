import { HttpgdViewer } from '../viewer';
import { getById } from '../utils'

interface ToolbarAction {
    id: string,
    keys: number[],
    altKey?: boolean,
    f: () => void,
}

export class ToolbarView {
    static readonly DELAY_FADE_OUT: number = 4000;
    private static readonly CSS_SHOW_DROPDOWN: string = "drop-open";

    private viewer: HttpgdViewer;

    private timoutFade?: ReturnType<typeof setTimeout>;

    private elemToolbar: HTMLElement;
    private elemContainer: HTMLElement;

    private dropdown: HTMLElement;
    private zoomLabel: HTMLElement;
    private pageLabel: HTMLElement;

    constructor(viewer: HttpgdViewer) {
        this.viewer = viewer;
        this.dropdown = getById("tb-more").parentElement;
        this.dropdown.onmouseenter = () => this.showDropdown();
        this.dropdown.onmouseleave = () => this.hideDropdown();

        this.zoomLabel = getById("tb-zlvl");
        this.pageLabel = getById("tb-pnum");

        this.elemToolbar = getById("toolbar");
        this.elemContainer = getById("container");

        this.timoutFade = setTimeout(() => this.fadeOut(), ToolbarView.DELAY_FADE_OUT);
        this.elemContainer.onmousemove = () => {
            this.elemToolbar.classList.remove("fade-out");
            clearTimeout(this.timoutFade);
            this.timoutFade = setTimeout(() => this.fadeOut(), ToolbarView.DELAY_FADE_OUT);
        }
        this.elemContainer.onmouseleave = () => this.fadeOut();
    }

    public registerActions(actions: ToolbarAction[]): void {
        const shortcuts: { [id: number]: ToolbarAction | undefined } = {};
        for (const a of actions) {
            getById(a.id).onclick = () => {
                a.f();
                this.hideDropdown();
            };
            if (a.keys) {
                for (const k of a.keys) {
                    shortcuts[k] = a;
                }
            }
        }

        window.addEventListener('keydown', (e) => {
            if (this.viewer.exportView.isVisible()) return;
            const a = shortcuts[e.keyCode];
            if (a && (!a.altKey || e.altKey)) {
                a.f();
                e.preventDefault();
                return;
            }
        });
    }

    private showDropdown() {
        this.dropdown.classList.add(ToolbarView.CSS_SHOW_DROPDOWN);
    }
    private hideDropdown() {
        this.dropdown.classList.remove(ToolbarView.CSS_SHOW_DROPDOWN);
    }

    public setZoomLabelText(s: string): void {
        this.zoomLabel.childNodes[0].nodeValue = s;
    }
    public setPageLabelText(s: string): void {
        this.pageLabel.childNodes[0].nodeValue = s;
    }

    private fadeOut() {
        this.elemToolbar.classList.add("fade-out");
    }
}