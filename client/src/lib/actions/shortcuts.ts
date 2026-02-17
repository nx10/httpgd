export interface Shortcut {
  key: string;
  altKey?: boolean;
  action: () => void;
}

export function shortcuts(node: HTMLElement, shortcuts: Shortcut[]) {
  function handleKeydown(e: KeyboardEvent) {
    // Don't trigger shortcuts when typing in input fields
    const target = e.target as HTMLElement;
    if (
      target.tagName === "INPUT" ||
      target.tagName === "TEXTAREA" ||
      target.tagName === "SELECT"
    ) {
      return;
    }

    // Don't trigger on export dialog
    if (document.querySelector("[data-dialog-overlay]")) {
      return;
    }

    for (const s of shortcuts) {
      const keyMatch = e.key === s.key;
      const altMatch = s.altKey ? e.altKey : !e.altKey;
      if (keyMatch && altMatch) {
        e.preventDefault();
        s.action();
        return;
      }
    }
  }

  window.addEventListener("keydown", handleKeydown);

  return {
    destroy() {
      window.removeEventListener("keydown", handleKeydown);
    },
    update(newShortcuts: Shortcut[]) {
      shortcuts = newShortcuts;
    },
  };
}
