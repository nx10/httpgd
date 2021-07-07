import { Httpgd } from './httpgd'
import { HttpgdPlotsResponse } from './types';
import { ExportView } from './views/exportView';
import { OverlayView } from './views/overlayView';
import { PlotView } from './views/plotView';

export class HttpgdViewer {
    static readonly COOLDOWN_DEVICE_INACTIVE: number = 1000;

    public httpgd: Httpgd;
    public plotView?: PlotView;
    public overlayView?: OverlayView;
    public exportView?: ExportView;

    private deviceInactiveDelayed?: ReturnType<typeof setTimeout>;

    public onDeviceActiveChange?: (deviceActive: boolean) => void;
    public onDisconnectedChange?: (disconnected: boolean) => void;
    public onIndexStringChange?: (indexString: string) => void;
    public onZoomStringChange?: (zoomString: string) => void;

    public constructor(host: string, token?: string, allowWebsockets?: boolean) {
        this.httpgd = new Httpgd(host, token, allowWebsockets);

        this.httpgd.onPlotsChanged((newState) => this.plotsChanged(newState));
        this.httpgd.onConnectionChange((newState) => this.connectionChanged(newState));
        this.httpgd.onDeviceActiveChanged((newState) => this.deviceActiveChanged(newState));
    }

    private plotsChanged(newState: HttpgdPlotsResponse): void {
        this.plotView?.updatePlots(newState);
        this.plotView?.update();
    }

    private connectionChanged(newState: boolean): void {
        if (newState) {
            this.overlayView?.show(OverlayView.TEXT_CONNECTION_LOST);
        } else {
            this.overlayView?.hide();
        }
    }

    private deviceActiveChanged(active: boolean): void {
        if (this.deviceInactiveDelayed) {
            clearTimeout(this.deviceInactiveDelayed);
        }
        if (!active) {
            this.deviceInactiveDelayed = setTimeout(
                () => this.overlayView?.show(OverlayView.TEXT_DEVICE_INACTIVE),
                HttpgdViewer.COOLDOWN_DEVICE_INACTIVE);
        } else {
            this.overlayView?.hide();
        }
    }

    public init(): void {

        this.plotView = new PlotView(this);
        this.overlayView = new OverlayView(this);
        this.exportView = new ExportView(this);
        
        this.httpgd.connect().then(() => {
            this.exportView.initRenderers();
        });

        this.plotView.toolbar.registerActions([
            {
                keys: [37, 40],
                f: () => this.plotView.prevPage(),
                id: "tb-left",
            },
            {
                keys: [39, 38],
                f: () => this.plotView.nextPage(),
                id: "tb-right",
            },
            {
                keys: [78],
                f: () => this.plotView.newestPage(),
                id: "tb-pnum",
            },
            {
                keys: [187],
                f: () => this.plotView.zoomIn(),
                id: "tb-plus",
            },
            {
                keys: [189],
                f: () => this.plotView.zoomOut(),
                id: "tb-minus",
            },
            {
                keys: [48],
                f: () => this.plotView.zoomReset(),
                id: "tb-zlvl",
            },
            {
                id: "tb-clear",
                altKey: true,
                keys: [68],
                f: () => this.plotView.clearPlots(),
            },
            {
                id: "tb-remove",
                keys: [46, 68],
                f: () => this.plotView.removePlot(),
            },
            {
                id: "tb-save-svg",
                keys: [83],
                f: () => this.plotView.downloadSVG(),
            },
            {
                id: "tb-save-png",
                keys: [80],
                f: () => this.plotView.downloadPNG(),
            },
            {
                id: "tb-copy-png",
                keys: [67],
                f: () => this.plotView.copyPNG(),
            },
            {
                id: "tb-history",
                keys: [72],
                f: () => this.plotView.sidebar.toggle(),
            },
            {
                id: "tb-export",
                keys: [69],
                f: () => this.exportView.show(),
            },
        ]);
        /*
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
                });*/
    }

    /*private plotsChanged(newState: HttpgdPlotsResponse, oldState?: HttpgdPlotsResponse): void {
        const url = this.httpgd.getPlotURL(newState.plots[newState.plots.length -1]);
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
            if (idx == 1)
                elem_card.classList.add("history-selected");
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
    public zoomOut(): void {
        if (this.scale - HttpgdViewer.SCALE_STEP > 0.05) {
            this.scale -= HttpgdViewer.SCALE_STEP;
        }
        this.onZoomStringChange?.(this.getZoomString());
        this.checkResize();
    }
    public zoomIn(): void {
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
        return Math.ceil( this.scale / HttpgdViewer.SCALE_DEFAULT * 100) + '%';
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

    
    public downloadPlotSVG(image: HTMLImageElement) {
        if (!this.navi.id()) return;
        fetch(image.src).then((response) => {
            return response.blob();
        }).then(blob => {
            downloadURL(URL.createObjectURL(blob), 'plot_'+this.navi.id()+'.svg');
        });
    }

    public downloadPlotPNG(image: HTMLImageElement) {
        if (!image) return;
        if (!this.navi.id()) return;
        imageTempCanvas(image, canvas => {
            const imgURI = canvas
                .toDataURL('image/png')
                .replace('image/png', 'image/octet-stream');
            downloadURL(imgURI, 'plot_'+this.navi.id()+'.png');
        });
    }

    public copyPlotPNG(image: HTMLImageElement) {
        if (!image) return;
        if (!this.navi.id()) return;
        if (!navigator.clipboard) return;
        imageTempCanvas(image, canvas => {
            canvas.toBlob(blob => { 
                if (blob) 
                    navigator.clipboard.write?.([new ClipboardItem({ 'image/png': blob })]) 
            });
        });
    }

    
    

    public id(): string | undefined {
        return this.navi.id();
    }

    public plot(id: string, renderer: string, width?: number, height?: number, zoom?: number, c?: string, download?: string): URL {
        return this.connection.api.plot_id(id, renderer, width, height, zoom, c, download);
    }*/
}