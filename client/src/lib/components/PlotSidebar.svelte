<script lang="ts">
  import { ScrollArea } from "$lib/components/ui/scroll-area";
  import SidebarItem from "./SidebarItem.svelte";
  import { plotsStore } from "$lib/stores/plots.svelte";
  import { uiStore } from "$lib/stores/ui.svelte";
  import { connectionStore } from "$lib/stores/connection.svelte";
  import { removePlot } from "$lib/httpgd/api";

  let scrollContainer: HTMLDivElement | undefined = $state();

  // Auto-scroll to selected item
  $effect(() => {
    const _page = plotsStore.page;
    if (!scrollContainer) return;
    // Find the selected element and scroll to it
    requestAnimationFrame(() => {
      const selected = scrollContainer?.querySelector("[data-selected=true]");
      if (selected) {
        selected.scrollIntoView({ behavior: "smooth", block: "center" });
      }
    });
  });

  async function handleRemove(plotId: string) {
    try {
      await removePlot(plotsStore.host, { id: plotId }, plotsStore.token);
    } catch {
      uiStore.showToast("Failed to remove plot", "error");
    }
  }
</script>

<div
  class="bg-sidebar flex h-full w-[20%] min-w-[120px] flex-col border-l transition-all duration-300 {uiStore.sidebarVisible
    ? 'translate-x-0'
    : 'translate-x-full'}"
  class:hidden={!uiStore.sidebarVisible}
>
  {#if !connectionStore.deviceActive}
    <div
      class="m-2 mb-0 flex items-center justify-center gap-1.5 rounded-md border border-amber-500/40 bg-amber-500/15 px-2 py-1.5 text-xs font-medium text-amber-600 dark:text-amber-400"
      role="status"
    >
      <span class="h-2 w-2 shrink-0 rounded-full bg-amber-500"></span>
      Device inactive
    </div>
  {/if}
  <ScrollArea class="min-h-0 flex-1">
    <div bind:this={scrollContainer} class="flex flex-col gap-2 p-2">
      {#each plotsStore.plots as plot (plot.id)}
        {@const isSelected = plot.id === plotsStore.currentPlotId}
        <div data-selected={isSelected}>
          <SidebarItem
            plotId={plot.id}
            selected={isSelected}
            onselect={() => plotsStore.setPage(plot.id)}
            onremove={() => handleRemove(plot.id)}
          />
        </div>
      {:else}
        <p class="py-8 text-center text-xs text-muted-foreground opacity-50">
          No plots yet
        </p>
      {/each}
    </div>
  </ScrollArea>
</div>
