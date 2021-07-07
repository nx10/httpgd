interface ToolbarEntry {
    title: string,
    shortcut?: string,
    id: string,
    content: string,
}

interface ToolbarData {
    main: ToolbarEntry[][],
    more: ToolbarEntry,
    dropdown: ToolbarEntry[],
}

const icon = (id: string): string => require("../assets/icons/" + id + ".svg");

export const data: ToolbarData = {
    main: [
        [
            {
                title: "Previous",
                shortcut: "&larr;",
                id: "tb-left",
                content: icon("arrow-left"),
            },
            {
                title: "Newest",
                shortcut: "N",
                id: "tb-pnum",
                content: "0/0",
            },
            {
                title: "Next",
                shortcut: "&rarr;",
                id: "tb-right",
                content: icon("arrow-right"),
            },
        ],
        [
            {
                title: "Zoom out",
                shortcut: "-",
                id: "tb-minus",
                content: icon("magnify-minus"),
            },
            {
                title: "Reset zoom",
                shortcut: "0",
                id: "tb-zlvl",
                content: "100%",
            },
            {
                title: "Zoom in",
                shortcut: "+",
                id: "tb-plus",
                content: icon("magnify-plus"),
            },
        ],
        [
            {
                title: "Delete",
                shortcut: "D",
                id: "tb-remove",
                content: icon("cross"),
            },
        ],
    ],
    more:
    {
        title: "More",
        id: "tb-more",
        content: icon("vdots"),
    },
    dropdown: [
        {
            title: "Download SVG",
            shortcut: "S",
            id: "tb-save-svg",
            content: icon("download"),
        },
        {
            title: "Download PNG",
            shortcut: "P",
            id: "tb-save-png",
            content: icon("image"),
        },
        {
            title: "Copy PNG",
            shortcut: "C",
            id: "tb-copy-png",
            content: icon("copy"),
        },
        {
            title: "Clear all plots",
            shortcut: "Alt+D",
            id: "tb-clear",
            content: icon("trash"),
        },
        {
            title: "Export",
            shortcut: "E",
            id: "tb-export",
            content: icon("export"),
        },
        {
            title: "Show history",
            shortcut: "H",
            id: "tb-history",
            content: icon("hamburger"),
        },
    ]
};