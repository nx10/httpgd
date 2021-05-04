import {HttpgdNavigator, HttpgdConnection, HttpgdPlots, HttpgdState, HttpgdRenderer} from './httpgd'
import {strcmp} from './utils'


export class HttpgdViewer {
    static readonly COOLDOWN_RESIZE: number = 200;
    static readonly SCALE_DEFAULT: number = 0.8;
    static readonly SCALE_STEP: number = HttpgdViewer.SCALE_DEFAULT / 12.0;

    private navi: HttpgdNavigator = new HttpgdNavigator();
    private plotUpid: number = -1;
    private scale: number = HttpgdViewer.SCALE_DEFAULT; // zoom level

    private connection: HttpgdConnection;
    private deviceActive: boolean = true;
    private image?: HTMLImageElement = undefined;
    private sidebar?: HTMLElement = undefined;

    public onDeviceActiveChange?: (deviceActive: boolean) => void;
    public onDisconnectedChange?: (disconnected: boolean) => void;
    public onIndexStringChange?: (indexString: string) => void;
    public onZoomStringChange?: (zoomString: string) => void;

    public renderers: HttpgdRenderer[] = [];

    public constructor(host: string, token?: string, allowWebsockets?: boolean) {
        this.connection = new HttpgdConnection(host, token, allowWebsockets);
        this.connection.remoteStateChanged = (remoteState: HttpgdState) => this.serverChanges(remoteState);
        this.connection.connectionChanged = (disconnected: boolean) => this.onDisconnectedChange?.(disconnected);
    }

    public init(image: HTMLImageElement, sidebar?: HTMLElement, exportSelect?: HTMLSelectElement): void {
        this.image = image;
        this.sidebar = sidebar;

        this.connection.open();
        this.checkResize();

        // Force reload on visibility change
        // Firefox otherwise shows a blank screen on tab change 
        document.addEventListener('visibilitychange', () => {
            if (!document.hidden) {
                this.updateImage('v');
            }
        }, false);

        this.onIndexStringChange?.(this.navi.indexStr());
        this.onZoomStringChange?.(this.getZoomString());

        console.log('initial update plots')
        this.updatePlots(true);

        this.connection.api.get_renderers().then((res) => {
            this.renderers = res.renderers.sort((a, b) => strcmp(a.name, b.name));
            this.renderers.sort((a, b) => strcmp(a.name, b.name)).forEach((r) => {
                const o = document.createElement("option");
                o.value = r.id;
                o.text = r.name + " (*"+ r.ext + ")";
                exportSelect.add(o);
            });
            exportSelect.value = "svg"
        });
    }

    private updatePlots(scroll: boolean = false) {
        this.connection.api.get_plots().then(plots => {
            this.navi.update(plots);
            this.onIndexStringChange?.(this.navi.indexStr());
            this.updateSidebar(plots, scroll);
            this.updateImage();
        })
    }

    private updateImage(c?: string) {
        if (!this.image) return;
        const n = this.navi.next(this.connection.api, this.plotUpid + (c ? c : ''));
        if (n) {
            console.log('update image');
            this.image.src = n;
        }
    }

    private updateSidebar(plots: HttpgdPlots, scroll: boolean = false) {
        if (!this.sidebar) return;

        //this.sidebar.innerHTML = '';

        let idx = 0;
        while (idx < this.sidebar.children.length) {
            if (idx >= plots.plots.length || this.sidebar.children[idx].getAttribute('data-pid') !== plots.plots[idx].id) {
                this.sidebar.removeChild(this.sidebar.children[idx]);
            } else {
                idx++;
            }
        }

        for (; idx < plots.plots.length; ++idx) {
            const p = plots.plots[idx];
            const elem_card = document.createElement("div");
            elem_card.setAttribute('data-pid', p.id);
            const elem_x = document.createElement("a");
            elem_x.innerHTML = "&#10006;"
            elem_x.onclick = () => {
                this.connection.api.get_remove_id(p.id);
                this.updatePlots();
            };
            const elem_img = document.createElement("img");
            elem_card.classList.add("history-item");
            elem_img.setAttribute('src', this.connection.api.svg_id(p.id).href);
            elem_card.onclick = () => {
                this.navi.jump_id(p.id);
                this.onIndexStringChange?.(this.navi.indexStr());
                this.updateImage();
            };
            elem_card.appendChild(elem_img);
            elem_card.appendChild(elem_x);
            this.sidebar.appendChild(elem_card);
        }

        if (scroll) {
            this.sidebar.scrollTop = this.sidebar.scrollHeight;
        }
    }

    // checks if there were server side changes
    private serverChanges(remoteState: HttpgdState): void {
        this.setDeviceActive(!remoteState.active);
        const lastUpid = this.plotUpid;
        this.plotUpid = remoteState.upid;
        if (lastUpid !== remoteState.upid)
            this.updatePlots(true);
    }

    private setDeviceActive(active: boolean): void {
        if (this.deviceActive !== active) {
            this.deviceActive = active;
            this.onDeviceActiveChange?.(active);
        }
    }

    // User interaction
    public zoomIn(): void {
        if (this.scale - HttpgdViewer.SCALE_STEP > 0.05) {
            this.scale -= HttpgdViewer.SCALE_STEP;
        }
        this.onZoomStringChange?.(this.getZoomString());
        this.checkResize();
    }
    public zoomOut(): void {
        this.scale += HttpgdViewer.SCALE_STEP;
        this.onZoomStringChange?.(this.getZoomString());
        this.checkResize();
    }
    public zoomReset(): void {
        this.scale = HttpgdViewer.SCALE_DEFAULT;
        this.onZoomStringChange?.(this.getZoomString());
        this.checkResize();
    }
    public getZoomString(): string {
        return Math.ceil(HttpgdViewer.SCALE_DEFAULT / this.scale * 100) + '%';
    }
    public navPrevious(): void {
        this.navi.navigate(-1);
        this.onIndexStringChange?.(this.navi.indexStr());
        this.updateImage();
    }
    public navNext(): void {
        this.navi.navigate(1);
        this.onIndexStringChange?.(this.navi.indexStr());
        this.updateImage();
    }
    public navNewest(): void {
        this.navi.jump(-1);
        this.onIndexStringChange?.(this.navi.indexStr());
        this.updateImage();
    }
    public navClear(): void {
        this.connection.api.get_clear();
        this.updatePlots();
    }
    public navRemove(): void {
        const id = this.navi.id();
        if (id) {
            this.connection.api.get_remove_id(id);
            this.updatePlots();
        }
    }

    private static downloadURL(url: string, filename?: string) {
        const dl = document.createElement('a');
        dl.href = url;
        if (filename) { dl.download = filename; }
        document.body.appendChild(dl);
        dl.click();
        document.body.removeChild(dl);
    }
    public downloadPlotSVG(image: HTMLImageElement) {
        if (!this.navi.id()) return;
        fetch(image.src).then((response) => {
            return response.blob();
        }).then(blob => {
            HttpgdViewer.downloadURL(URL.createObjectURL(blob), 'plot_'+this.navi.id()+'.svg');
        });
    }

    private static imageTempCanvas(image: HTMLImageElement, fn: (canvas: HTMLCanvasElement) => void) {
        const canvas = document.createElement('canvas');
        document.body.appendChild(canvas);
        const rect = image.getBoundingClientRect();
        canvas.width = rect.width;
        canvas.height = rect.height;
        const ctx = canvas.getContext('2d');
        if (!ctx) return;
        ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
        fn(canvas);
        document.body.removeChild(canvas);
    }

    public downloadPlotPNG(image: HTMLImageElement) {
        if (!image) return;
        if (!this.navi.id()) return;
        HttpgdViewer.imageTempCanvas(image, canvas => {
            const imgURI = canvas
                .toDataURL('image/png')
                .replace('image/png', 'image/octet-stream');
            HttpgdViewer.downloadURL(imgURI, 'plot_'+this.navi.id()+'.png');
        });
    }

    public copyPlotPNG(image: HTMLImageElement) {
        if (!image) return;
        if (!this.navi.id()) return;
        if (!navigator.clipboard) return;
        HttpgdViewer.imageTempCanvas(image, canvas => {
            canvas.toBlob(blob => { 
                if (blob) 
                    navigator.clipboard.write?.([new ClipboardItem({ 'image/png': blob })]) 
            });
        });
    }

    public checkResize() {
        if (!this.image) return;
        const rect = this.image.getBoundingClientRect();
        this.navi.resize(rect.width * this.scale, rect.height * this.scale);
        this.updateImage();
    }

    // this is called by window.addEventListener('resize', ...)
    private resizeBlocked: boolean = false;
    public resize() {
        if (this.resizeBlocked) return;
        this.resizeBlocked = true;
        setTimeout(() => {
            this.checkResize();
            this.resizeBlocked = false;
        }, HttpgdViewer.COOLDOWN_RESIZE);
    }
    

    public id(): string {
        return this.navi.id();
    }

    public plot(id: string, renderer: string, width?: number, height?: number, c?: string, download?: string): URL {
        return this.connection.api.plot_id(id, renderer, width, height, c, download);
    }
}