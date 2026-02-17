<script lang="ts">
  import X from "@lucide/svelte/icons/x";
  import type { PlotId } from "$lib/httpgd/types";
  import { plotsStore } from "$lib/stores/plots.svelte";
  import { getPlotUrl } from "$lib/httpgd/api";

  interface Props {
    plotId: PlotId;
    selected: boolean;
    onselect: () => void;
    onremove: () => void;
  }

  let { plotId, selected, onselect, onremove }: Props = $props();

  const thumbUrl = $derived(
    getPlotUrl(
      plotsStore.host,
      { id: plotId },
      plotsStore.token,
      plotsStore.upid,
    ),
  );
</script>

<div
  class="group relative w-full cursor-pointer overflow-hidden rounded-md border shadow-sm transition-all hover:shadow-md {selected
    ? 'ring-primary ring-2'
    : ''}"
  role="button"
  tabindex="0"
  onclick={onselect}
  onkeydown={(e: KeyboardEvent) => {
    if (e.key === "Enter" || e.key === " ") {
      e.preventDefault();
      onselect();
    }
  }}
>
  <div class="bg-white">
    <img
      src={thumbUrl}
      alt="Plot thumbnail"
      class="h-[12vw] min-h-[60px] w-full object-cover"
    />
  </div>
  <button
    type="button"
    class="bg-background/80 text-muted-foreground hover:text-destructive absolute top-1 right-1 rounded-sm p-0.5 opacity-0 transition-opacity group-focus-within:opacity-100 group-hover:opacity-100"
    onclick={(e: MouseEvent) => {
      e.stopPropagation();
      onremove();
    }}
    aria-label="Remove plot"
  >
    <X class="h-4 w-4" />
  </button>
</div>
