
import { HttpgdViewer } from '../viewer';
import { HttpgdPlotsResponse } from '../types';
import { getById, safeScrollTo } from '../utils'

export class SidebarView {
    private viewer: HttpgdViewer;

    //private elemContainer: HTMLElement;
    private elemPlotView: HTMLElement;
    private elemSidebar: HTMLElement;

    constructor(viewer: HttpgdViewer, hidden?: boolean) {
        this.viewer = viewer;
        this.elemPlotView = getById("plotview");
        this.elemSidebar = getById("sidebar");

        if (hidden) {
            this.elemSidebar.classList.add('notransition', 'nohist');
            this.elemPlotView.classList.add('notransition', 'nohist');
            setTimeout(() => {
                this.elemSidebar.classList.remove('notransition');
                this.elemPlotView.classList.remove('notransition');
            }, 300);
        }
    }

    /**
     * Loops throgh the plot list and adds or removes sidebar thumbnail cards as needed.
     * 
     * @param plots 
     */
    public update(plots: HttpgdPlotsResponse): void {
        //this.sidebar.innerHTML = '';

        let idx = 0;
        while (idx < this.elemSidebar.children.length) {
            if (idx >= plots.plots.length || this.elemSidebar.children[idx].getAttribute('data-pid') !== plots.plots[idx].id) {
                this.elemSidebar.removeChild(this.elemSidebar.children[idx]);
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
                this.viewer.httpgd.removePlot({ id: p.id })
                this.viewer.httpgd.updatePlots();
            };
            const elem_img = document.createElement("img");
            elem_card.classList.add("history-item");
            elem_img.setAttribute('src', this.viewer.httpgd.getPlotURL({ id: p.id }));
            elem_card.onclick = () => this.viewer.plotView.setPage(p.id);
            elem_card.appendChild(elem_img);
            elem_card.appendChild(elem_x);
            this.elemSidebar.appendChild(elem_card);
        }
    }

    public setSelected(plotId: string): void {

        let activeCard: Element; 

        for (let i = 0; i < this.elemSidebar.children.length; ++i) {
            const card = this.elemSidebar.children[i];
            if (card.getAttribute('data-pid') === plotId) {
                card.classList.add("history-selected");
                activeCard = card;
            } else {
                card.classList.remove("history-selected");
            }
        }

        if (activeCard) { // scroll to card
            const bb_card = activeCard.getBoundingClientRect();
            const y_card = bb_card.y + this.elemSidebar.scrollTop;
            const h_card = bb_card.height;
            const h_sidebar = this.elemSidebar.getBoundingClientRect().height;
            const h = y_card - (h_sidebar/2 - h_card/2);
            safeScrollTo(this.elemSidebar, {
                top: h,
                behavior: 'smooth'
            }); 
        }
    }

    public toggle(): void {
        this.elemSidebar.classList.toggle('nohist');
        this.elemPlotView.classList.toggle('nohist');
        setTimeout(() => this.viewer.plotView.update(), 300);
    }

    public hideWithoutAnimation(): void {
        this.elemSidebar.classList.add('notransition', 'nohist');
        this.elemPlotView.classList.add('notransition', 'nohist');
        this.viewer.plotView.update();
        setTimeout(() => {
            this.elemSidebar.classList.remove('notransition');
            this.elemPlotView.classList.remove('notransition');
        }, 300);
    }
}