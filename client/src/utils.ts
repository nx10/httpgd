export function getById<T extends HTMLElement>(id: string): T {
    const el = document.getElementById(id);
    if (!el) {
        throw new ReferenceError(id + " is not defined");
    }
    return el as T;
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

export function downloadURL(url: string, filename?: string, tab?: boolean): void {
    const dl = document.createElement('a');
    dl.href = url;
    if (filename) { dl.download = filename; }
    if (tab) { dl.target = '_blank'; }
    document.body.appendChild(dl);
    dl.click();
    document.body.removeChild(dl);
}

export function copyClipboardPNG(url: string): void {
    if (!navigator.clipboard?.write) {
        console.warn("No clipboard API support!");
        return;
    }

    fetch(url).then(res => res.blob()).then(blob => {
        if (!blob)
            return;
        navigator.clipboard.write([new ClipboardItem(
            Object.defineProperty({}, "image/png", {
                value: blob,
                enumerable: true
            })
        )]);
    })
}

export function imageTempCanvas(image: HTMLImageElement, fn: (canvas: HTMLCanvasElement) => Promise<void>): void {
    const canvas = document.createElement('canvas');
    canvas.style.display = "none";
    document.body.appendChild(canvas);
    const rect = image.getBoundingClientRect();
    canvas.width = rect.width;
    canvas.height = rect.height;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;
    ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
    fn(canvas).finally(() => document.body.removeChild(canvas));
}

export function downloadImgSVG(image: HTMLImageElement, filename: string): void {
    fetch(image.src).then((response) => {
        return response.blob();
    }).then(blob => {
        downloadURL(URL.createObjectURL(blob), filename);
    });
}

export function downloadImgPNG(image: HTMLImageElement, filename: string): void {
    imageTempCanvas(image, async canvas => {
        const imgURI = canvas
            .toDataURL('image/png')
            .replace('image/png', 'image/octet-stream');
        downloadURL(imgURI, filename);
    });
}

export function copyImgSVGasPNG(image: HTMLImageElement): void {
    if (!navigator.clipboard?.write) {
        console.warn("No clipboard API support!");
        return;
    }
    imageTempCanvas(image, async canvas => {
        const blob = await new Promise<Blob | null>((resolve) => canvas.toBlob(resolve));
        if (!blob)
            return;
        return await navigator.clipboard.write([new ClipboardItem(
            Object.defineProperty({}, blob.type, {
                value: blob,
                enumerable: true
            })
        )]);
    });

}

export function validNumberInput(input: HTMLInputElement, min: number, max: number): boolean {
    const s = input.value;
    if (!s.match(/^\d+$/)) return false;
    const v = parseInt(s);
    return (v >= min) && (v <= max);
}

export function setCssClass(element: HTMLElement, set: boolean, cssClass: string): boolean {
    if (set) {
        element.classList.add(cssClass);
    } else {
        element.classList.remove(cssClass);
    }
    return set;
}

const supportsNativeSmoothScroll = 'scrollBehavior' in document.documentElement.style;

export function safeScrollTo(elem: Element, options: ScrollToOptions): void {
    if (supportsNativeSmoothScroll) {
        elem.scrollTo(options);
    } else {
        elem.scrollTo(options.left ? options.left : 0, options.top ? options.top : 0);
    }
}