const FADE_DELAY = 4000;

export function autofade(node: HTMLElement) {
  let timer: ReturnType<typeof setTimeout>;

  function fadeOut() {
    // Don't fade if a popover/dropdown is open (bits-ui portals)
    if (document.querySelector('[data-slot="dropdown-menu-content"]')) {
      resetFade();
      return;
    }
    node.classList.add("toolbar-faded");
  }

  function resetFade() {
    node.classList.remove("toolbar-faded");
    clearTimeout(timer);
    timer = setTimeout(fadeOut, FADE_DELAY);
  }

  // Start fade timer
  timer = setTimeout(fadeOut, FADE_DELAY);

  // Track mouse movement on the parent container
  const container = node.parentElement;
  if (container) {
    container.addEventListener("mousemove", resetFade);
    container.addEventListener("mouseleave", fadeOut);
  }

  return {
    destroy() {
      clearTimeout(timer);
      if (container) {
        container.removeEventListener("mousemove", resetFade);
        container.removeEventListener("mouseleave", fadeOut);
      }
    },
  };
}
