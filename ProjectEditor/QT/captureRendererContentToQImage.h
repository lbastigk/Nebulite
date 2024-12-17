#include <SDL2/SDL.h>
#include <QImage>

QImage captureRendererContentToQImage(SDL_Renderer *renderer, int rendererWidth, int rendererHeight, int imageWidth, int imageHeight) {
    // Create a surface for pixel data
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, rendererWidth, rendererHeight, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) {
        qWarning("Failed to create surface: %s", SDL_GetError());
        return QImage();
    }

    // Ensure there's a valid render target
    SDL_Texture *currentTarget = SDL_GetRenderTarget(renderer);
    if (!currentTarget) {
        currentTarget = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            rendererWidth, rendererHeight
        );

        if (!currentTarget) {
            SDL_FreeSurface(surface);
            return QImage();
        }

        // Set the temporary render target
        if (SDL_SetRenderTarget(renderer, currentTarget) != 0) {
            SDL_DestroyTexture(currentTarget);
            SDL_FreeSurface(surface);
            return QImage();
        }
    }

    // Create a QImage initialized to black using the allocated pixels
    QImage image((uchar*)surface->pixels, rendererWidth, rendererHeight, QImage::Format_RGBA8888);
    image.fill(Qt::black); // Initialize all pixels to black

    // Copy rendered pixels into the memory pointed to by 'pixels'
    if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch) != 0) {
        qWarning("Failed to read pixels: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return QImage();
    }

    // Scale the image to the desired size
    QImage scaledImage = image.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QImage finalImage = scaledImage.copy();

    // Free the original surface memory
    SDL_FreeSurface(surface);

    // Return the scaled QImage
    return finalImage;
}

