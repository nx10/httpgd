export type CopyResult = "copied" | "downloaded" | "failed";

/**
 * Render an image element to a PNG blob at its full natural resolution.
 */
function toPNGBlob(image: HTMLImageElement): Promise<Blob> {
  return new Promise((resolve, reject) => {
    const canvas = document.createElement("canvas");
    canvas.width = image.naturalWidth;
    canvas.height = image.naturalHeight;
    const ctx = canvas.getContext("2d");
    if (!ctx) {
      reject(new Error("Failed to get canvas context"));
      return;
    }
    ctx.drawImage(image, 0, 0);
    canvas.toBlob((blob) => {
      if (blob) resolve(blob);
      else reject(new Error("Failed to create blob"));
    });
  });
}

/**
 * Trigger a browser file download from a URL.
 * When `openInTab` is true, opens the URL in a new tab instead.
 */
export function downloadURL(
  url: string,
  filename?: string,
  openInTab?: boolean,
): void {
  const a = document.createElement("a");
  a.href = url;
  if (filename) a.download = filename;
  if (openInTab) a.target = "_blank";
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
}

/**
 * Download a blob as a file.
 */
function downloadBlob(blob: Blob, filename: string): void {
  const url = URL.createObjectURL(blob);
  downloadURL(url, filename);
  URL.revokeObjectURL(url);
}

/**
 * Download a remote image (fetched from `src`) as a file.
 */
export async function downloadImgSVG(
  src: string,
  filename: string,
): Promise<void> {
  const res = await fetch(src);
  const blob = await res.blob();
  downloadBlob(blob, filename);
}

/**
 * Render a displayed image to PNG and download it at full resolution.
 */
export async function downloadImgPNG(
  image: HTMLImageElement,
  filename: string,
): Promise<void> {
  downloadBlob(await toPNGBlob(image), filename);
}

/**
 * Copy a displayed image to the clipboard as PNG.
 *
 * Uses the async Clipboard API with a promise-based `ClipboardItem` for
 * Safari compatibility. Falls back to downloading the PNG if the Clipboard
 * API is unavailable or blocked (e.g. in embedded webviews).
 *
 * Returns `"copied"` on clipboard success, `"downloaded"` if the fallback
 * was used, or `"failed"` if both failed.
 */
export async function copyPNG(
  image: HTMLImageElement,
  fallbackFilename = "plot.png",
): Promise<CopyResult> {
  const blobPromise = toPNGBlob(image);

  if (
    window.isSecureContext &&
    navigator.clipboard?.write &&
    window.ClipboardItem
  ) {
    try {
      const item = new ClipboardItem({ "image/png": blobPromise });
      await navigator.clipboard.write([item]);
      return "copied";
    } catch {
      // Fall through to download fallback
    }
  }

  try {
    downloadBlob(await blobPromise, fallbackFilename);
    return "downloaded";
  } catch {
    return "failed";
  }
}
