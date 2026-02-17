<script lang="ts">
  import * as Dialog from "$lib/components/ui/dialog";
  import { Button } from "$lib/components/ui/button";
  import { uiStore } from "$lib/stores/ui.svelte";
  import { plotsStore } from "$lib/stores/plots.svelte";
  import { connectionStore } from "$lib/stores/connection.svelte";
  import { getPlotUrl } from "$lib/httpgd/api";
  import { downloadURL } from "$lib/utils/download";
  const MIN_SIZE = 1;
  const MAX_SIZE = 10000;
  const MIN_ZOOM = 1;
  const MAX_ZOOM = 10000;

  let inputWidth = $state("800");
  let inputHeight = $state("600");
  let inputZoom = $state("100");
  let selectedFormat = $state("svg");

  const hasPlot = $derived(plotsStore.currentPlotId !== null);

  function getRenderersList() {
    const r = connectionStore.renderers;
    if (!Array.isArray(r)) return [];
    return r.slice().sort((a, b) => a.name.localeCompare(b.name));
  }

  function validNum(s: string, min: number, max: number): boolean {
    if (!/^\d+$/.test(s)) return false;
    const v = parseInt(s);
    return v >= min && v <= max;
  }

  const validWidth = $derived(validNum(inputWidth, MIN_SIZE, MAX_SIZE));
  const validHeight = $derived(validNum(inputHeight, MIN_SIZE, MAX_SIZE));
  const validZoom = $derived(validNum(inputZoom, MIN_ZOOM, MAX_ZOOM));
  const allValid = $derived(validWidth && validHeight && validZoom && hasPlot);

  const previewUrl = $derived.by(() => {
    const plotId = plotsStore.currentPlotId;
    if (!plotId || !validWidth || !validHeight || !validZoom) return null;
    return getPlotUrl(
      plotsStore.host,
      {
        id: plotId,
        width: Math.min(parseInt(inputWidth), MAX_SIZE),
        height: Math.min(parseInt(inputHeight), MAX_SIZE),
        zoom: Math.max(parseInt(inputZoom) / 100, 0.01),
      },
      plotsStore.token,
      plotsStore.upid,
    );
  });

  function getSelectedRenderer() {
    return connectionStore.renderers.find((r) => r.id === selectedFormat);
  }

  function handleDownload() {
    const plotId = plotsStore.currentPlotId;
    if (!plotId || !allValid) return;
    const renderer = getSelectedRenderer();
    if (!renderer) return;
    const url = getPlotUrl(
      plotsStore.host,
      {
        id: plotId,
        width: Math.min(parseInt(inputWidth), MAX_SIZE),
        height: Math.min(parseInt(inputHeight), MAX_SIZE),
        zoom: Math.max(parseInt(inputZoom) / 100, 0.01),
        renderer: renderer.id,
        download: `plot_${plotId}${renderer.ext}`,
      },
      plotsStore.token,
      plotsStore.upid,
    );
    downloadURL(url);
  }

  function handleOpen() {
    const plotId = plotsStore.currentPlotId;
    if (!plotId || !allValid) return;
    const renderer = getSelectedRenderer();
    if (!renderer) return;
    const url = getPlotUrl(
      plotsStore.host,
      {
        id: plotId,
        width: Math.min(parseInt(inputWidth), MAX_SIZE),
        height: Math.min(parseInt(inputHeight), MAX_SIZE),
        zoom: Math.max(parseInt(inputZoom) / 100, 0.01),
        renderer: renderer.id,
      },
      plotsStore.token,
      plotsStore.upid,
    );
    downloadURL(url, undefined, true);
  }

  function handleOpenChange(open: boolean) {
    uiStore.exportDialogOpen = open;
  }
</script>

<Dialog.Root open={uiStore.exportDialogOpen} onOpenChange={handleOpenChange}>
  <Dialog.Content class="flex max-h-[85vh] max-w-[85vw] flex-col">
    <Dialog.Header>
      <Dialog.Title>Export Plot</Dialog.Title>
      {#if !hasPlot}
        <Dialog.Description>No plot selected.</Dialog.Description>
      {/if}
    </Dialog.Header>

    <div class="flex flex-1 items-center justify-center overflow-hidden py-4">
      {#if hasPlot && previewUrl}
        <img
          src={previewUrl}
          alt="Export preview"
          class="bg-muted max-h-full max-w-full rounded border object-contain shadow-sm"
        />
      {:else}
        <p class="text-muted-foreground">No plot selected.</p>
      {/if}
    </div>

    <div class="flex flex-wrap items-center justify-between gap-4">
      <div class="flex items-center gap-2">
        <span class="text-muted-foreground text-sm">Size:</span>
        <input
          type="number"
          min={MIN_SIZE}
          max={MAX_SIZE}
          bind:value={inputWidth}
          disabled={!hasPlot}
          class="bg-background h-9 w-20 rounded-md border px-3 text-sm {validWidth
            ? ''
            : 'border-destructive bg-destructive/10'}"
        />
        <span class="text-muted-foreground">&times;</span>
        <input
          type="number"
          min={MIN_SIZE}
          max={MAX_SIZE}
          bind:value={inputHeight}
          disabled={!hasPlot}
          class="bg-background h-9 w-20 rounded-md border px-3 text-sm {validHeight
            ? ''
            : 'border-destructive bg-destructive/10'}"
        />
        <span class="text-muted-foreground">~</span>
        <input
          type="number"
          min={MIN_ZOOM}
          max={MAX_ZOOM}
          bind:value={inputZoom}
          disabled={!hasPlot}
          class="bg-background h-9 w-20 rounded-md border px-3 text-sm {validZoom
            ? ''
            : 'border-destructive bg-destructive/10'}"
        />
        <span class="text-muted-foreground text-sm">%</span>
      </div>

      <div class="flex items-center gap-2">
        <select
          bind:value={selectedFormat}
          disabled={!hasPlot}
          class="bg-background text-foreground h-9 rounded-md border px-3 text-sm"
        >
          {#each getRenderersList() as r (r.id)}
            <option value={r.id} title={r.descr}>
              {r.name} (*{r.ext})
            </option>
          {/each}
        </select>
        <Button variant="secondary" onclick={handleOpen} disabled={!allValid}>
          Open
        </Button>
        <Button onclick={handleDownload} disabled={!allValid}>Download</Button>
      </div>
    </div>
  </Dialog.Content>
</Dialog.Root>
