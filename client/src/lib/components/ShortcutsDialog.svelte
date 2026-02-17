<script lang="ts">
  import * as Dialog from "$lib/components/ui/dialog";
  import { uiStore } from "$lib/stores/ui.svelte";

  const shortcuts = [
    { keys: ["\u2190", "\u2193"], action: "Previous plot" },
    { keys: ["\u2192", "\u2191"], action: "Next plot" },
    { keys: ["N"], action: "Newest plot" },
    { keys: ["+", "="], action: "Zoom in" },
    { keys: ["-"], action: "Zoom out" },
    { keys: ["0"], action: "Reset zoom" },
    { keys: ["D"], action: "Delete plot" },
    { keys: ["Alt+D"], action: "Clear all plots" },
    { keys: ["S"], action: "Download SVG" },
    { keys: ["P"], action: "Download PNG" },
    { keys: ["C"], action: "Copy PNG" },
    { keys: ["E"], action: "Export dialog" },
    { keys: ["H"], action: "Toggle history" },
    { keys: ["T"], action: "Toggle theme" },
    { keys: ["?"], action: "Show shortcuts" },
  ];

  function handleOpenChange(open: boolean) {
    uiStore.shortcutsDialogOpen = open;
  }
</script>

<Dialog.Root open={uiStore.shortcutsDialogOpen} onOpenChange={handleOpenChange}>
  <Dialog.Content class="flex max-h-[85vh] max-w-sm flex-col">
    <Dialog.Header>
      <Dialog.Title>Keyboard shortcuts</Dialog.Title>
    </Dialog.Header>
    <div
      class="grid grid-cols-[1fr_auto] gap-x-6 gap-y-1.5 overflow-y-auto pr-3 text-sm"
    >
      {#each shortcuts as s (s.action)}
        <span class="text-muted-foreground">{s.action}</span>
        <span class="flex justify-end gap-1">
          {#each s.keys as key, i (key)}
            {#if i > 0}<span class="text-muted-foreground">/</span>{/if}
            <kbd
              class="bg-muted text-muted-foreground inline-flex min-w-[1.5rem] items-center justify-center rounded border px-1.5 py-0.5 font-mono text-xs"
              >{key}</kbd
            >
          {/each}
        </span>
      {/each}
    </div>
  </Dialog.Content>
</Dialog.Root>
