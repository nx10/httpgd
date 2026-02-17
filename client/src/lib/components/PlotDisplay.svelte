<script lang="ts">
  import ImageOff from "@lucide/svelte/icons/image-off";
  import { plotsStore } from "$lib/stores/plots.svelte";

  interface Props {
    imageRef?: HTMLImageElement | null;
  }

  let { imageRef = $bindable(null) }: Props = $props();

  let container: HTMLDivElement;
  let width = $state(0);
  let height = $state(0);
  let loading = $state(false);

  const plotUrl = $derived.by(() => {
    if (width === 0 || height === 0) return null;
    return plotsStore.getPlotImageUrl(width, height);
  });

  const hasPlot = $derived(plotsStore.currentPlotId !== null);

  $effect(() => {
    if (!container) return;
    const observer = new ResizeObserver((entries) => {
      const entry = entries[0];
      if (!entry) return;
      width = entry.contentRect.width;
      height = entry.contentRect.height;
    });
    observer.observe(container);
    return () => observer.disconnect();
  });

  function onLoad() {
    loading = false;
  }

  function onLoadStart() {
    if (plotUrl) loading = true;
  }
</script>

<div
  bind:this={container}
  class="relative flex h-full w-full items-center justify-center overflow-hidden"
>
  {#if hasPlot && plotUrl}
    <img
      bind:this={imageRef}
      src={plotUrl}
      alt="Plot"
      crossorigin="anonymous"
      class="max-h-full max-w-full object-contain transition-opacity duration-150"
      class:opacity-80={loading}
      onload={onLoad}
      onloadstart={onLoadStart}
    />
  {:else}
    <div class="text-muted-foreground flex flex-col items-center gap-3">
      <ImageOff class="h-12 w-12 opacity-30" />
      <p class="text-sm opacity-50">Waiting for plots</p>
    </div>
  {/if}
</div>
