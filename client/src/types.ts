
export interface HttpgdBackend {
    host: string,
    token?: string
}

// API response objects

export interface HttpgdStateResponse {
    upid: number,
    hsize: number,
    active: boolean
}

export interface HttpgdIdResponse {
    id: string
}

export interface HttpgdPlotsResponse {
    state: HttpgdStateResponse,
    plots: HttpgdIdResponse[]
}

export interface HttpgdRendererResponse {
    id: string, 
    mime: string, 
    ext: string, 
    name: string, 
    type: string, 
    bin: boolean
}

export interface HttpgdRenderersResponse {
    renderers: HttpgdRendererResponse[]
}

// API request objects

export interface HttpgdPlotRequest {
    id?: string, 
    renderer?: string, 
    width?: number, 
    height?: number, 
    zoom?: number, 
    download?: string
}


export interface HttpgdRemoveRequest {
    id: string, 
}
