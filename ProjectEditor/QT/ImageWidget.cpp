#include "ImageWidget.h"

#include <iostream>


void dumpImageAsBinary(const QImage &image, const QString &fileName) {
    // Create a QBuffer to store the image in a byte array (acting as an in-memory file)
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);

    // Save the QImage to the buffer in PNG format (you can change the format)
    if (!image.save(&buffer, "PNG")) {
        std::cerr << "Failed to save image to byte array!" << std::endl;
        return;
    }

    // Open the output file in binary mode
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        // Write the byte array from the buffer to the file
        file.write(buffer.data());
        file.close();
    } else {
        std::cerr << "Failed to open file for writing!" << std::endl;
    }
}

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent), label(new QLabel(this)) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
}

void ImageWidget::updateImage() {
    dumpImageAsBinary(currentImage,"textImageInUpdateImage.png");
    label->setPixmap(QPixmap::fromImage(currentImage)); //Program crashes here
}

void ImageWidget::mouseEvent(QMouseEvent *event){
    // Get the cursor position relative to the widget
    QPoint cursorPos = event->pos();

    // Make sure the cursor is within the bounds of the QImage
    if (currentImage.rect().contains(cursorPos)) {
        // Get the pixel color from the QImage at the cursor position
        QColor pixelColor = currentImage.pixelColor(cursorPos);

        // Output the pixel position and color
        // qDebug() << "Cursor Position:" << cursorPos;
        // qDebug() << "Pixel Color:" << pixelColor.name(); // Output the color in hexadecimal
    }
}

void ImageWidget::convertSdlToImage(SDL_Renderer *renderer, int rendererWidth, int rendererHeight, int imageWidth, int imageHeight) {
    // Create a surface for pixel data
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, rendererWidth, rendererHeight, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) {
        qWarning("Failed to create surface: %s", SDL_GetError());
        return;
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
            return;
        }

        // Set the temporary render target
        if (SDL_SetRenderTarget(renderer, currentTarget) != 0) {
            SDL_DestroyTexture(currentTarget);
            SDL_FreeSurface(surface);
            return;
        }
    }

    // Create a QImage initialized to black using the allocated pixels
    QImage image((uchar*)surface->pixels, rendererWidth, rendererHeight, QImage::Format_RGBA8888);
    image.fill(Qt::black); // Initialize all pixels to black

    // Copy rendered pixels into the memory pointed to by 'pixels'
    if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch) != 0) {
        qWarning("Failed to read pixels: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    // Scale the image to the desired size
    currentImage = image.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation).copy();

    if (currentImage.isNull()) {
        std::cerr << "Error: currentImage is null or empty!" << std::endl;
        return;
    }

    // Free the original surface memory
    SDL_FreeSurface(surface);
}



