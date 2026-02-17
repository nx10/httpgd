interface ToastState {
  message: string;
  type: "success" | "error";
}

class UiStore {
  sidebarVisible = $state(true);
  exportDialogOpen = $state(false);
  shortcutsDialogOpen = $state(false);
  overlayText = $state<string | null>(null);
  overlayAction = $state<{ label: string; callback: () => void } | null>(null);
  toast = $state<ToastState | null>(null);
  private toastTimer: ReturnType<typeof setTimeout> | undefined;

  toggleSidebar() {
    this.sidebarVisible = !this.sidebarVisible;
  }

  showOverlay(text: string, action?: { label: string; callback: () => void }) {
    this.overlayText = text;
    this.overlayAction = action ?? null;
  }

  hideOverlay() {
    this.overlayText = null;
    this.overlayAction = null;
  }

  openExportDialog() {
    this.exportDialogOpen = true;
  }

  closeExportDialog() {
    this.exportDialogOpen = false;
  }

  showToast(message: string, type: "success" | "error" = "success") {
    if (this.toastTimer) clearTimeout(this.toastTimer);
    this.toast = { message, type };
    this.toastTimer = setTimeout(() => {
      this.toast = null;
    }, 3000);
  }

  dismissToast() {
    if (this.toastTimer) clearTimeout(this.toastTimer);
    this.toast = null;
  }
}

export const uiStore = new UiStore();
