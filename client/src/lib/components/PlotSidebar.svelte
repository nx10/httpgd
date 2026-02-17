<script lang="ts">
  import { ScrollArea } from "$lib/components/ui/scroll-area";
  import SidebarItem from "./SidebarItem.svelte";
  import { plotsStore } from "$lib/stores/plots.svelte";
  import { uiStore } from "$lib/stores/ui.svelte";
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
  class="bg-sidebar h-full w-[20%] min-w-[120px] border-l transition-all duration-300 {uiStore.sidebarVisible
    ? 'translate-x-0'
    : 'translate-x-full'}"
  class:hidden={!uiStore.sidebarVisible}
>
  <ScrollArea class="h-full">
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
