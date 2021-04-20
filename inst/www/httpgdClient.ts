import { HttpgdViewer } from './httpgd'

function getById(id: string): HTMLElement {
    const el = getById(id);
    if (!el) {
        throw new ReferenceError(id + " is not defined");
    }
    return el;
}

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

var httpgdViewer = new HttpgdViewer(
    sparams.get("host") || window.location.host,
    sparams.get("token") || undefined,
    sparams.has("ws") ? (sparams.get("ws") != "0") : true
);

interface HttpgdAction {
    id: string,
    fun: () => void,
    altKey?: boolean,
    keys?: number[],
    onclickId?: string,
}

window.onload = function () {

    const dropElem = getById("tb-more").parentElement as HTMLElement;
    dropElem.onmouseenter = () => {
        dropElem.classList.add('drop-open');
    };
    dropElem.onmouseleave = () => {
        dropElem.classList.remove('drop-open');
    };
    function closeDropdown() {
        dropElem.classList.remove('drop-open');
    };

    httpgdViewer.onZoomStringChange = (s) => {
        getById("tb-zlvl").childNodes[0].nodeValue = s;
    }
    httpgdViewer.onIndexStringChange = (s) => {
        getById("tb-pnum").childNodes[0].nodeValue = s;
    }
    httpgdViewer.onDisconnectedChange = (d) => {
        getById("overlay-text").innerText = "Connection lost.";
        getById("overlay").style.display = d ? "inline" : "none";
    }
    var httpgd_inactive_delayed: number | undefined;
    httpgdViewer.onDeviceActiveChange = (d) => {
        if (!d) {
            clearTimeout(httpgd_inactive_delayed);
            getById("overlay").style.display = "none";
        }
        else {
            httpgd_inactive_delayed = setTimeout(() => {
                getById("overlay-text").innerText = "Device inactive.";
                getById("overlay").style.display = "inline";
            }, 1000);
        }
    }
    window.addEventListener('resize', () => httpgdViewer.resize());

    const elem_img = getById("drawing") as HTMLImageElement;
    httpgdViewer.init(elem_img, getById("sidebar"));


    function toggleSidebar() {
        getById("sidebar").classList.toggle('nohist')
        getById("plotview").classList.toggle('nohist')
        setTimeout(() => httpgdViewer.checkResize(), 300);
        closeDropdown();
    }

    const actions: HttpgdAction[] = [
        {
            id: "navPrevious",
            keys: [37, 40],
            fun: () => httpgdViewer.navPrevious(),
            onclickId: "tb-left",
        },
        {
            id: "navNext",
            keys: [39, 38],
            fun: () => httpgdViewer.navNext(),
            onclickId: "tb-right",
        },
        {
            id: "navNewest",
            keys: [78],
            fun: () => httpgdViewer.navNewest(),
            onclickId: "tb-pnum",
        },
        {
            id: "zoomIn",
            keys: [187],
            fun: () => httpgdViewer.zoomIn(),
            onclickId: "tb-plus",
        },
        {
            id: "zoomOut",
            keys: [189],
            fun: () => httpgdViewer.zoomOut(),
            onclickId: "tb-minus",
        },
        {
            id: "navClear",
            altKey: true,
            keys: [68],
            fun: () => {
                httpgdViewer.navClear();
                closeDropdown();
            },
            onclickId: "tb-clear",
        },
        {
            id: "navRemove",
            keys: [46, 68],
            fun: () => httpgdViewer.navRemove(),
            onclickId: "tb-remove",
        },
        {
            id: "downloadPlotSVG",
            keys: [83],
            fun: () => {
                httpgdViewer.downloadPlotSVG(elem_img);
                closeDropdown();
            },
            onclickId: "tb-save-svg",
        },
        {
            id: "downloadPlotPNG",
            keys: [80],
            fun: () => {
                httpgdViewer.downloadPlotPNG(elem_img);
                closeDropdown();
            },
            onclickId: "tb-save-png",
        },
        {
            id: "zoomReset",
            keys: [48],
            fun: () => httpgdViewer.zoomReset(),
            onclickId: "tb-zlvl",
        },
        {
            id: "copyPlotPNG",
            keys: [67],
            fun: () => {
                httpgdViewer.copyPlotPNG(elem_img);
                closeDropdown();
            },
            onclickId: "tb-copy-png",
        },
        {
            id: "toggleSidebar",
            keys: [72],
            fun: () => toggleSidebar(),
            onclickId: "tb-history",
        },
        {
            id: "export",
            keys: [72],
            fun: () => {
                modal.style.display = "block";
            },
            onclickId: "tb-export",
        }
    ];

    // Build keyboard shortcut dictionary
    const shortcuts: { [id: number]: HttpgdAction | undefined } = {};
    for (const a of actions) {
        if (a.onclickId) {
            getById(a.onclickId).onclick = a.fun;
        }
        if (a.keys) {
            for (const k of a.keys) {
                shortcuts[k] = a;
            }
        }
    }

    window.addEventListener('keydown', (e) => {
        const a = shortcuts[e.keyCode];
        if (a && (!a.altKey || e.altKey)) {
            a.fun();
            e.preventDefault();
            return;
        }
    });


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

    // Get the modal
    var modal = getById("myModal");


    // Get the <span> element that closes the modal
    var span = document.getElementsByClassName("modal-close")[0] as HTMLElement;

    // When the user clicks on <span> (x), close the modal
    span.onclick = function () {
        modal.style.display = "none";
    }

    // When the user clicks anywhere outside of the modal, close it
    window.onclick = function (event: MouseEvent) {
        if (event.target == modal) {
            modal.style.display = "none";
        }
    }
}