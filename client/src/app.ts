import { HttpgdViewer } from "./viewer";

const sparams = new URL(window.location.href).searchParams;

export default {
    viewer: new HttpgdViewer(
        sparams.get("host") || window.location.host,
        sparams.get("token") || undefined,
        sparams.has("ws") ? (sparams.get("ws") != "0") : true,
        sparams.has("sidebar") ? (sparams.get("sidebar") == "0") : false
    )
};