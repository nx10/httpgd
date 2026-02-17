<script lang="ts">
  import { onMount } from "svelte";
  import Toast from "$lib/components/Toast.svelte";
  import { ModeWatcher } from "mode-watcher";
  import * as Tooltip from "$lib/components/ui/tooltip";
  import PlotDisplay from "$lib/components/PlotDisplay.svelte";
  import Toolbar from "$lib/components/Toolbar.svelte";
  import PlotSidebar from "$lib/components/PlotSidebar.svelte";
  import ExportDialog from "$lib/components/ExportDialog.svelte";
  import Overlay from "$lib/components/Overlay.svelte";
  import ShortcutsDialog from "$lib/components/ShortcutsDialog.svelte";
  import { Connection } from "$lib/httpgd/connection";
  import { removePlot, clearPlots } from "$lib/httpgd/api";
  import { parseConnectionParams } from "$lib/httpgd/url";
  import { connectionStore } from "$lib/stores/connection.svelte";
  import { plotsStore } from "$lib/stores/plots.svelte";
  import { uiStore } from "$lib/stores/ui.svelte";
  import { shortcuts } from "$lib/actions/shortcuts";
  import { downloadImgSVG, downloadImgPNG } from "$lib/utils/download";
  import { copyPNG } from "$lib/utils/clipboard";
  import { toggleMode } from "mode-watcher";

  let plotImage: HTMLImageElement | null = $state(null);
  let connection: Connection;
  let deviceInactiveTimer: ReturnType<typeof setTimeout> | undefined;

  onMount(() => {
    const params = parseConnectionParams();
    plotsStore.configure(params.host, params.token);

    if (params.sidebarHidden) {
      uiStore.sidebarVisible = false;
    }

    connection = new Connection(
      params.host,
      params.token,
      params.allowWebsockets,
      {
        onPlotsChanged(plots) {
          plotsStore.updatePlots(plots);
        },
        onConnectionChanged(connected) {
          connectionStore.setConnected(connected);
          if (!connected) {
            uiStore.showOverlay("Connection lost.", {
              label: "Reconnect",
              callback: () => handleReconnect(),
            });
          } else {
            uiStore.hideOverlay();
          }
        },
        onDeviceActiveChanged(active) {
          connectionStore.setDeviceActive(active);
          if (deviceInactiveTimer) clearTimeout(deviceInactiveTimer);
          if (!active) {
            deviceInactiveTimer = setTimeout(() => {
              uiStore.showOverlay("Device inactive.");
            }, 1000);
          } else {
            uiStore.hideOverlay();
          }
        },
        onRenderersChanged(renderers) {
          connectionStore.setRenderers(renderers);
        },
      },
    );

    connection.connect().then(() => {
      if (connection.info) {
        connectionStore.setInfo(connection.info);
        console.log("Connected to " + connection.info.version);
      }
    });

    return () => {
      connection.disconnect();
      if (deviceInactiveTimer) clearTimeout(deviceInactiveTimer);
    };
  });

  async function handleReconnect() {
    uiStore.showOverlay("Reconnecting\u2026");
    await connection.reconnect();
    if (connection.info) {
      connectionStore.setInfo(connection.info);
    }
  }

  async function handleDownloadSVG() {
    if (!plotImage?.src) return;
    try {
      await downloadImgSVG(plotImage.src, "plot.svg");
    } catch {
      uiStore.showToast("Failed to download SVG", "error");
    }
  }

  async function handleDownloadPNG() {
    if (!plotImage) return;
    try {
      await downloadImgPNG(plotImage, "plot.png");
    } catch {
      uiStore.showToast("Failed to download PNG", "error");
    }
  }

  async function handleCopyPNG() {
    if (!plotImage) return;
    try {
      const ok = await copyPNG(plotImage);
      if (ok) {
        uiStore.showToast("PNG copied to clipboard");
      } else {
        uiStore.showToast("Failed to copy PNG", "error");
      }
    } catch {
      uiStore.showToast("Failed to copy PNG", "error");
    }
  }

  async function handleRemove() {
    const plotId = plotsStore.currentPlotId;
    if (!plotId) return;
    try {
      await removePlot(plotsStore.host, { id: plotId }, plotsStore.token);
    } catch {
      uiStore.showToast("Failed to remove plot", "error");
    }
  }

  async function handleClear() {
    try {
      await clearPlots(plotsStore.host, plotsStore.token);
    } catch {
      uiStore.showToast("Failed to clear plots", "error");
    }
  }
</script>

<ModeWatcher />
<Toast />

<Tooltip.Provider>
  <div
    class="flex h-full w-full"
    use:shortcuts={[
      { key: "ArrowLeft", action: () => plotsStore.prevPage() },
      { key: "ArrowDown", action: () => plotsStore.prevPage() },
      { key: "ArrowRight", action: () => plotsStore.nextPage() },
      { key: "ArrowUp", action: () => plotsStore.nextPage() },
      { key: "n", action: () => plotsStore.newestPage() },
      { key: "+", action: () => plotsStore.zoomIn() },
      { key: "=", action: () => plotsStore.zoomIn() },
      { key: "-", action: () => plotsStore.zoomOut() },
      { key: "0", action: () => plotsStore.zoomReset() },
      { key: "d", altKey: true, action: handleClear },
      { key: "d", action: handleRemove },
      { key: "Delete", action: handleRemove },
      { key: "s", action: handleDownloadSVG },
      { key: "p", action: handleDownloadPNG },
      { key: "c", action: handleCopyPNG },
      { key: "h", action: () => uiStore.toggleSidebar() },
      { key: "e", action: () => uiStore.openExportDialog() },
      { key: "t", action: toggleMode },
      {
        key: "?",
        action: () => (uiStore.shortcutsDialogOpen = true),
      },
    ]}
  >
    <div class="relative flex-1 overflow-hidden">
      <Toolbar
        ondownloadsvg={handleDownloadSVG}
        ondownloadpng={handleDownloadPNG}
        oncopypng={handleCopyPNG}
        onremove={handleRemove}
        onclear={handleClear}
      />
      <PlotDisplay bind:imageRef={plotImage} />
    </div>

    <PlotSidebar />
  </div>

  <ExportDialog />
  <ShortcutsDialog />
  <Overlay />
</Tooltip.Provider>
