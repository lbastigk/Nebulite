QImage captureRendererContentToQImage(SDL_Renderer *renderer, int width, int height) {
    // Create an SDL surface with a pixel format that matches what SDL uses
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) {
        qWarning("Failed to create surface for QImage conversion. SDL_Error: %s", SDL_GetError());
        return QImage();
    }

    // Get the pixel format of the renderer's current target
    SDL_RendererInfo rendererInfo;
    SDL_GetRendererInfo(renderer, &rendererInfo);
    Uint32 renderFormat = rendererInfo.texture_formats[0];  // Default format used by the renderer

    // Read pixels from the texture, not from the renderer
    if (SDL_RenderReadPixels(renderer, NULL, renderFormat, surface->pixels, surface->pitch) != 0) {
        qWarning("Failed to read pixels: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return QImage();
    }

    // Convert the SDL surface into a QImage
    QImage image((uchar*)surface->pixels, width, height, surface->pitch, QImage::Format_RGBA8888);
    QImage result = image.copy(); // Make a copy to manage the data safely outside SDL's lifecycle

    // Free the SDL surface
    SDL_FreeSurface(surface);
    return result;
}
