/// <reference types="svelte" />
/// <reference types="vite/client" />

declare module "*.svg" {
  const src: string;
  export default src;
}
