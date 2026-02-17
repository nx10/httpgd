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

export async function downloadImgSVG(
  imageSrc: string,
  filename: string,
): Promise<void> {
  const res = await fetch(imageSrc);
  const blob = await res.blob();
  const url = URL.createObjectURL(blob);
  downloadURL(url, filename);
  URL.revokeObjectURL(url);
}

export async function downloadImgPNG(
  image: HTMLImageElement,
  filename: string,
): Promise<void> {
  const canvas = document.createElement("canvas");
  const rect = image.getBoundingClientRect();
  canvas.width = rect.width;
  canvas.height = rect.height;
  const ctx = canvas.getContext("2d");
  if (!ctx) return;
  ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
  const dataUrl = canvas
    .toDataURL("image/png")
    .replace("image/png", "image/octet-stream");
  downloadURL(dataUrl, filename);
}
