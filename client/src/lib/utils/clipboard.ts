export async function copyPNG(image: HTMLImageElement): Promise<boolean> {
  if (!navigator.clipboard?.write) {
    console.warn("Clipboard API not supported");
    return false;
  }

  const canvas = document.createElement("canvas");
  const rect = image.getBoundingClientRect();
  canvas.width = rect.width;
  canvas.height = rect.height;
  const ctx = canvas.getContext("2d");
  if (!ctx) return false;
  ctx.drawImage(image, 0, 0, canvas.width, canvas.height);

  const blob = await new Promise<Blob | null>((resolve) =>
    canvas.toBlob(resolve),
  );
  if (!blob) return false;

  await navigator.clipboard.write([new ClipboardItem({ [blob.type]: blob })]);
  return true;
}
