import "./style/style.scss";
//import { HttpgdViewer } from "./httpgdViewer";
import App from "./app" ;

/*const ASSET_PLOT_NONE: string = require('./assets/plot-none.svg');

const sparams = new URL(window.location.href).searchParams;

const elem_sidebar = getById("sidebar");
const elem_plotview = getById("plotview");

if (sparams.has("sidebar") && sparams.get("sidebar")) {
    elem_sidebar.classList.add('notransition', 'nohist');
    elem_plotview.classList.add('notransition', 'nohist');
    setTimeout(() => {
        elem_sidebar.classList.remove('notransition');
        elem_plotview.classList.remove('notransition');
    }, 300);
}

interface HttpgdAction {
    id: string,
    fun: () => void,
    altKey?: boolean,
    keys?: number[],
    onclickId?: string,
}
*/
window.onload = function () {
    App.viewer.init();

    /*const dropElem = getById("tb-more").parentElement as HTMLElement;
    dropElem.onmouseenter = () => {
        dropElem.classList.add('drop-open');
    };
    dropElem.onmouseleave = () => {
        dropElem.classList.remove('drop-open');
    };
    function closeDropdown() {
        dropElem.classList.remove('drop-open');
    };

    App.viewer.onZoomStringChange = (s) => {
        getById("tb-zlvl").childNodes[0].nodeValue = s;
    }
    App.viewer.onIndexStringChange = (s) => {
        getById("tb-pnum").childNodes[0].nodeValue = s;
    }


    const elem_img = getById("drawing") as HTMLImageElement;
    const exp_format = getById("ie-format") as HTMLSelectElement;
    App.viewer.init(elem_img, getById("sidebar"), exp_format);


    
*/
    
    /*??????????const modal_close = document.getElementsByClassName("modal-close")[0] as HTMLElement;
    modal_close.onclick = function () {
        modal.style.display = "none";
    }
    // When the user clicks anywhere outside of the modal, close it
    window.onmousedown = function (event: MouseEvent) {
        if (event.target == modal) {
            modal.style.display = "none";
        }
    }*/
/*
    const exp_image = getById("exp-image") as HTMLImageElement;

    const exp_width = getById("ie-width") as HTMLInputElement;
    const exp_height = getById("ie-height") as HTMLInputElement;
    const exp_scale = getById("ie-scale") as HTMLInputElement; 
    const exp_btn_open = getById("ie-btn-open") as HTMLButtonElement;
    const exp_btn_download = getById("ie-btn-download") as HTMLButtonElement;
    

    exp_btn_download.onclick = (ev) => {
        const w = Math.min(parseInt(exp_width.value), 10000);
        const h = Math.min(parseInt(exp_height.value), 10000);
        const z = Math.max(parseInt(exp_scale.value) / 100, 0.01);
        const r = App.viewer.renderers.find((r) => r.id == exp_format.value);
        const url = App.viewer.plot(App.viewer.id(), r.id, w, h, z, undefined, "plot_" + App.viewer.id() + r.ext).href;
        downloadURL(url);
    };
    
    exp_btn_open.onclick = (ev) => {
        const w = Math.min(parseInt(exp_width.value), 10000);
        const h = Math.min(parseInt(exp_height.value), 10000);
        const z = Math.max(parseInt(exp_scale.value) / 100, 0.01);
        const r = App.viewer.renderers.find((r) => r.id == exp_format.value);
        const url = App.viewer.plot(App.viewer.id(), r.id, w, h, z, undefined).href;
        window.open(url, '_blank');
    };

    function exp_change() {
        const id = App.viewer.id();
        if (!id)
        {
            exp_image.src = ASSET_PLOT_NONE;
            return;
        }
        const w = Math.min(parseInt(exp_width.value), 10000);
        const h = Math.min(parseInt(exp_height.value), 10000);
        const z = Math.max(parseInt(exp_scale.value) / 100, 0.01);
        exp_image.src = App.viewer.plot(id, "svg", w, h, z).href;
    }
    exp_change();

    exp_width.addEventListener('input', exp_change);
    exp_height.addEventListener('input', exp_change);
    exp_scale.addEventListener('input', exp_change);

    
*/
    // Build keyboard shortcut dictionary
    /*????const shortcuts: { [id: number]: HttpgdAction | undefined } = {};
    for (const a of actions) {
        if (a.onclickId) {
            getById(a.onclickId).onclick = a.fun;
        }
        if (a.keys) {
            for (const k of a.keys) {
                shortcuts[k] = a;
            }
        }
    }*/

    /*????const modal = getById("exp-modal");
    window.addEventListener('keydown', (e) => {
        if (modal.style.display && 
            modal.style.display !== "none") return;
        const a = shortcuts[e.keyCode];
        if (a && (!a.altKey || e.altKey)) {
            a.fun();
            e.preventDefault();
            return;
        }
    });*/

/*
    const elem_tbt = getById("toolbar");
    const elem_cnt = getById("container");
    const TB_FADE_TIMEOUT = 4000;

    var httpgd_tb_fade = setTimeout(() => elem_tbt.classList.add("fade-out"), TB_FADE_TIMEOUT);
    elem_cnt.onmousemove = function () {
        elem_tbt.classList.remove("fade-out");
        clearTimeout(httpgd_tb_fade);
        httpgd_tb_fade = setTimeout(() => elem_tbt.classList.add("fade-out"), TB_FADE_TIMEOUT);
    }
    elem_cnt.onmouseleave = function (mouseEvent) {
        elem_tbt.classList.add("fade-out");
    }
*/
}