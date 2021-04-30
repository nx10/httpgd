// from: https://github.com/Microsoft/TypeScript/issues/26728

interface ClipboardItem {
}

declare var ClipboardItem: {
  prototype: ClipboardItem;
  new(objects: Record<string, Blob>): ClipboardItem;
};

interface Clipboard {
  read?(): Promise<Array<ClipboardItem>>;

  write?(items: Array<ClipboardItem>): Promise<void>;
}