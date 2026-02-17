<script lang="ts">
  import { uiStore } from "$lib/stores/ui.svelte";
</script>

{#if uiStore.toast}
  <div
    class="fixed right-4 bottom-4 z-50 flex items-center gap-2 rounded-md border px-4 py-3 text-sm shadow-lg {uiStore
      .toast.type === 'error'
      ? 'border-red-200 bg-red-50 text-red-900 dark:border-red-800 dark:bg-red-950 dark:text-red-100'
      : 'border-green-200 bg-green-50 text-green-900 dark:border-green-800 dark:bg-green-950 dark:text-green-100'}"
  >
    <span>{uiStore.toast.message}</span>
    <button
      type="button"
      class="ml-2 opacity-50 hover:opacity-100"
      onclick={() => uiStore.dismissToast()}
      aria-label="Dismiss">&times;</button
    >
  </div>
{/if}
