<script lang="ts">
  import ChevronLeft from "@lucide/svelte/icons/chevron-left";
  import ChevronRight from "@lucide/svelte/icons/chevron-right";
  import ZoomOut from "@lucide/svelte/icons/zoom-out";
  import ZoomIn from "@lucide/svelte/icons/zoom-in";
  import Trash2 from "@lucide/svelte/icons/trash-2";
  import Download from "@lucide/svelte/icons/download";
  import Image from "@lucide/svelte/icons/image";
  import Copy from "@lucide/svelte/icons/copy";
  import X from "@lucide/svelte/icons/x";
  import FileOutput from "@lucide/svelte/icons/file-output";
  import PanelRight from "@lucide/svelte/icons/panel-right";
  import EllipsisVertical from "@lucide/svelte/icons/ellipsis-vertical";
  import Sun from "@lucide/svelte/icons/sun";
  import Moon from "@lucide/svelte/icons/moon";
  import Keyboard from "@lucide/svelte/icons/keyboard";
  import { Button } from "$lib/components/ui/button";
  import * as Tooltip from "$lib/components/ui/tooltip";
  import * as DropdownMenu from "$lib/components/ui/dropdown-menu";
  import { Separator } from "$lib/components/ui/separator";
  import { plotsStore } from "$lib/stores/plots.svelte";
  import { uiStore } from "$lib/stores/ui.svelte";
  import { autofade } from "$lib/actions/autofade";
  import { toggleMode, mode } from "mode-watcher";

  interface Props {
    ondownloadsvg: () => void;
    ondownloadpng: () => void;
    oncopypng: () => void;
    onremove: () => void;
    onclear: () => void;
  }

  let { ondownloadsvg, ondownloadpng, oncopypng, onremove, onclear }: Props =
    $props();
</script>

<div
  use:autofade
  class="bg-background/90 absolute top-2 right-2 z-10 flex items-center gap-0.5 rounded-lg border p-1 shadow-sm backdrop-blur-sm transition-all duration-500"
>
  <!-- Navigation -->
  <Tooltip.Root>
    <Tooltip.Trigger>
      {#snippet child({ props })}
        <Button
          {...props}
          variant="ghost"
          size="icon"
          class="h-8 w-8"
          onclick={() => plotsStore.prevPage()}
        >
          <ChevronLeft class="h-4 w-4" />
        </Button>
      {/snippet}
    </Tooltip.Trigger>
    <Tooltip.Content><p>Previous (&larr;)</p></Tooltip.Content>
  </Tooltip.Root>

  <Tooltip.Root>
    <Tooltip.Trigger>
      {#snippet child({ props })}
        <Button
          {...props}
          variant="ghost"
          size="sm"
          class="h-8 min-w-[3rem] px-2 text-xs font-bold"
          onclick={() => plotsStore.newestPage()}
        >
          {plotsStore.pageLabel}
        </Button>
      {/snippet}
    </Tooltip.Trigger>
    <Tooltip.Content><p>Newest (N)</p></Tooltip.Content>
  </Tooltip.Root>

  <Tooltip.Root>
    <Tooltip.Trigger>
      {#snippet child({ props })}
        <Button
          {...props}
          variant="ghost"
          size="icon"
          class="h-8 w-8"
          onclick={() => plotsStore.nextPage()}
        >
          <ChevronRight class="h-4 w-4" />
        </Button>
      {/snippet}
    </Tooltip.Trigger>
    <Tooltip.Content><p>Next (&rarr;)</p></Tooltip.Content>
  </Tooltip.Root>

  <Separator orientation="vertical" class="mx-1 h-6" />

  <!-- Zoom -->
  <Tooltip.Root>
    <Tooltip.Trigger>
      {#snippet child({ props })}
        <Button
          {...props}
          variant="ghost"
          size="icon"
          class="h-8 w-8"
          onclick={() => plotsStore.zoomOut()}
        >
          <ZoomOut class="h-4 w-4" />
        </Button>
      {/snippet}
    </Tooltip.Trigger>
    <Tooltip.Content><p>Zoom out (-)</p></Tooltip.Content>
  </Tooltip.Root>

  <Tooltip.Root>
    <Tooltip.Trigger>
      {#snippet child({ props })}
        <Button
          {...props}
          variant="ghost"
          size="sm"
          class="h-8 min-w-[3rem] px-2 text-xs font-bold"
          onclick={() => plotsStore.zoomReset()}
        >
          {plotsStore.zoomLabel}
        </Button>
      {/snippet}
    </Tooltip.Trigger>
    <Tooltip.Content><p>Reset zoom (0)</p></Tooltip.Content>
  </Tooltip.Root>

  <Tooltip.Root>
    <Tooltip.Trigger>
      {#snippet child({ props })}
        <Button
          {...props}
          variant="ghost"
          size="icon"
          class="h-8 w-8"
          onclick={() => plotsStore.zoomIn()}
        >
          <ZoomIn class="h-4 w-4" />
        </Button>
      {/snippet}
    </Tooltip.Trigger>
    <Tooltip.Content><p>Zoom in (+)</p></Tooltip.Content>
  </Tooltip.Root>

  <Separator orientation="vertical" class="mx-1 h-6" />

  <!-- Delete -->
  <Tooltip.Root>
    <Tooltip.Trigger>
      {#snippet child({ props })}
        <Button
          {...props}
          variant="ghost"
          size="icon"
          class="hover:text-destructive h-8 w-8"
          onclick={onremove}
        >
          <X class="h-4 w-4" />
        </Button>
      {/snippet}
    </Tooltip.Trigger>
    <Tooltip.Content><p>Delete (D)</p></Tooltip.Content>
  </Tooltip.Root>

  <!-- More menu -->
  <DropdownMenu.Root>
    <DropdownMenu.Trigger>
      {#snippet child({ props })}
        <Button {...props} variant="ghost" size="icon" class="h-8 w-8">
          <EllipsisVertical class="h-4 w-4" />
        </Button>
      {/snippet}
    </DropdownMenu.Trigger>
    <DropdownMenu.Content align="end" class="w-48">
      <DropdownMenu.Item onclick={ondownloadsvg}>
        <Download class="mr-2 h-4 w-4" />
        Download SVG
        <DropdownMenu.Shortcut>S</DropdownMenu.Shortcut>
      </DropdownMenu.Item>
      <DropdownMenu.Item onclick={ondownloadpng}>
        <Image class="mr-2 h-4 w-4" />
        Download PNG
        <DropdownMenu.Shortcut>P</DropdownMenu.Shortcut>
      </DropdownMenu.Item>
      <DropdownMenu.Item onclick={oncopypng}>
        <Copy class="mr-2 h-4 w-4" />
        Copy PNG
        <DropdownMenu.Shortcut>C</DropdownMenu.Shortcut>
      </DropdownMenu.Item>
      <DropdownMenu.Separator />
      <DropdownMenu.Item onclick={onclear} class="text-destructive">
        <Trash2 class="mr-2 h-4 w-4" />
        Clear all
        <DropdownMenu.Shortcut>Alt+D</DropdownMenu.Shortcut>
      </DropdownMenu.Item>
      <DropdownMenu.Separator />
      <DropdownMenu.Item onclick={() => uiStore.openExportDialog()}>
        <FileOutput class="mr-2 h-4 w-4" />
        Export
        <DropdownMenu.Shortcut>E</DropdownMenu.Shortcut>
      </DropdownMenu.Item>
      <DropdownMenu.Item onclick={() => uiStore.toggleSidebar()}>
        <PanelRight class="mr-2 h-4 w-4" />
        History
        <DropdownMenu.Shortcut>H</DropdownMenu.Shortcut>
      </DropdownMenu.Item>
      <DropdownMenu.Separator />
      <DropdownMenu.Item onclick={toggleMode}>
        {#if mode.current === "dark"}
          <Sun class="mr-2 h-4 w-4" />
          Light mode
        {:else}
          <Moon class="mr-2 h-4 w-4" />
          Dark mode
        {/if}
        <DropdownMenu.Shortcut>T</DropdownMenu.Shortcut>
      </DropdownMenu.Item>
      <DropdownMenu.Item onclick={() => (uiStore.shortcutsDialogOpen = true)}>
        <Keyboard class="mr-2 h-4 w-4" />
        Shortcuts
        <DropdownMenu.Shortcut>?</DropdownMenu.Shortcut>
      </DropdownMenu.Item>
    </DropdownMenu.Content>
  </DropdownMenu.Root>
</div>

<style>
  :global(.toolbar-faded) {
    opacity: 0;
    transform: translateY(-100px);
    transition:
      opacity 0.5s,
      transform 0.5s step-end 0.5s;
  }
</style>
