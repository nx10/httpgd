<script lang="ts">
  import { uiStore } from "$lib/stores/ui.svelte";
</script>

{#if uiStore.overlayText}
  <div
    class="fixed inset-0 z-50 flex flex-col items-center justify-center gap-6 bg-black/60 backdrop-blur-sm"
  >
    <p class="text-4xl font-light text-white drop-shadow-lg">
      {uiStore.overlayText}
    </p>
    {#if uiStore.overlayAction}
      <button
        type="button"
        class="cursor-pointer rounded-md border border-white/30 px-6 py-2 text-sm font-medium text-white transition-colors hover:bg-white/20"
        onclick={uiStore.overlayAction.callback}
      >
        {uiStore.overlayAction.label}
      </button>
    {/if}
  </div>
{/if}
