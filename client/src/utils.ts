export function getById(id: string): HTMLElement {
    const el = document.getElementById(id);
    if (!el) {
        throw new ReferenceError(id + " is not defined");
    }
    return el;
}

export function strcmp(a: string, b: string): number {
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

export function downloadURL(url: string, filename?: string) {
    const dl = document.createElement('a');
    dl.href = url;
    if (filename) { dl.download = filename; }
    document.body.appendChild(dl);
    dl.click();
    document.body.removeChild(dl);
}
