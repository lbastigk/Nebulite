//------------------------------------------
// Includes

// External
#include "SDL3_image/SDL_image.h"
#include "stb_image_write.h"

// Nebulite
#include "Core/Renderer.hpp"
#include "Core/RenderObject.hpp"
#include "Interaction/Invoke.hpp"
#include "Module/Domain/Renderer/General.hpp"
#include "Nebulite.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

//------------------------------------------
// Update

Constants::Event General::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event General::envLoad(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        // no name provided, load empty env
        domain.deserialize("{}");
        return Constants::Event::Success;
    }
    auto const fileName = Utility::StringHandler::recombineArgs(args.subspan(1));
    if (!Utility::IO::FileManagement::fileExists(fileName)) {
        domain.capture.error.println("File ", fileName, " does not exist! Loading an empty environment.");
        domain.deserialize("{}");
        return Constants::Event::Warning;
    }
    domain.deserialize(fileName);
    return Constants::Event::Success;
}

Constants::Event General::envDeload() const {
    domain.purgeObjects();
    domain.purgeTextures();
    return Constants::Event::Success;
}

Constants::Event General::spawn(int const argc, char** argv) const {
    if (argc > 1) {
        // Using all args, allowing for whitespaces in the link and in the following functioncalls:
        // e.g.: spawn Planets/sun.jsonc|set text.str This is a sun
        std::string const linkOrObject = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);

        // Create object with link to globalspace
        auto* ro = new Core::RenderObject(domain.capture);
        ro->deserialize(linkOrObject);

        // Append to renderer.
        // Renderer manages the RenderObjects lifetime
        domain.append(ro);
    } else {
        domain.capture.error.println("No RenderObject name provided!");
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    return Constants::Event::Success;
}

Constants::Event General::setResolution(int const argc, char** argv) const {
    int w = 1000;
    int h = 1000;
    uint8_t scalar = 1;
    if (argc > 1) {
        w = std::stoi(argv[1]);
    }
    if (argc > 2) {
        h = std::stoi(argv[2]);
    }
    if (argc > 3) {
        if (int const signedScalar = std::stoi(argv[3]); signedScalar > 0) {
            scalar = static_cast<uint8_t>(signedScalar);
        } else {
            scalar = 1;
        }
    }
    domain.changeWindowSize(w, h, scalar);
    return Constants::Event::Success;
}

Constants::Event General::setFPS(int const argc, char** argv) const {
    // Standard value for no argument
    uint16_t fps = 60;
    if (argc == 2) {
        int const fpsSigned = std::stoi(argv[1]);
        fps = static_cast<uint16_t>(fpsSigned);

        // Constrain fps to reasonable values
        if (fpsSigned < 1)
            fps = 1;
        if (fpsSigned > 10000)
            fps = 10000;
    }
    domain.setTargetFPS(fps);
    return Constants::Event::Success;
}

Constants::Event General::showFPS(int const argc, char** argv) const {
    if (argc < 2) {
        domain.toggleFps(true);
    } else {
        if (!strcmp(argv[1], "on")) {
            domain.toggleFps(true);
        } else if (!strcmp(argv[1], "off")) {
            domain.toggleFps(false);
        } else {
            // unknown arg
            return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
        }
    }
    return Constants::Event::Success;
}

Constants::Event General::cam_move(int const argc, char** argv) const {
    if (argc < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (argc > 3) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    int const dx = std::stoi(argv[1]);
    int const dy = std::stoi(argv[2]);
    domain.moveCam(dx, dy);
    return Constants::Event::Success;
}

Constants::Event General::cam_set(int const argc, char** argv) const {
    if (argc == 3) {
        int const x = std::stoi(argv[1]);
        int const y = std::stoi(argv[2]);
        domain.setCam(x, y);
        return Constants::Event::Success;
    }
    if (argc == 4) {
        if (!strcmp(argv[3], "c")) {
            int const x = std::stoi(argv[1]);
            int const y = std::stoi(argv[2]);
            domain.setCam(x, y, true);
            return Constants::Event::Success;
        }
        // unknown arg
        return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
    }
    if (argc > 4) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }
    return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
}

Constants::Event General::snapshot(int const argc, char** argv) const {
    if (argc > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    static std::string fileName;
    fileName = argc == 2 ? argv[1] : "./Resources/Snapshots/snapshot.png";
    auto snapshotFunction = [&] {
        // Get current window/render target size
        auto const window = domain.getSdlWindow();
        auto const renderer = domain.getSdlRenderer();
        int width, height;
        if (window) {
            // Normal windowed mode
            SDL_GetWindowSize(window, &width, &height);
        } else {
            // Headless mode - get renderer output size
            SDL_GetCurrentRenderOutputSize(renderer, &width, &height);
        }

        // Create surface to capture pixels
        SDL_Rect const fullScreenRect = {0, 0, width, height};
        auto const surface = SDL_RenderReadPixels(renderer, &fullScreenRect);
        if (!surface) {
            domain.capture.error.println("Failed to read pixels for snapshot: ", SDL_GetError());
            SDL_DestroySurface(surface);
            return;
        }

        // Create directory if it doesn't exist
        std::string directory = fileName.substr(0, fileName.find_last_of("/\\"));

        // Edge case: check if link contains no directory:
        if (fileName.find_last_of("/\\") == std::string::npos) {
            directory = "./Resources/Snapshots";
            fileName = directory + "/" + fileName;
        }

        if (!directory.empty()) {
            // Create directory using C++17 filesystem
            try {
                std::filesystem::create_directories(directory);
            } catch (std::exception const& e) {
                domain.capture.error.println("Warning: Could not create directory ", directory, ": ", e.what());
                // Continue anyway - maybe directory already exists
            }
        }

        // Save surface as PNG
        if (int const result = IMG_SavePNG(surface, fileName.c_str()); result != 0 && SDL_GetError()[0] != '\0') {
            domain.capture.error.println("Failed to save snapshot!");
            return;
        }

        // Cleanup
        SDL_DestroySurface(surface);
    };
    domain.addPostRenderCallback(snapshotFunction);
    return Constants::Event::Success;
}

namespace {
auto constexpr base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64_encode(uint8_t const* data, size_t const& len) {
    std::string out;
    // NOLINTNEXTLINE
    out.reserve(((len + 2) / 3) * 4);

    int val = 0, valb = -6;
    for (size_t i = 0; i < len; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            // NOLINTNEXTLINE
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    // NOLINTNEXTLINE
    if (valb > -6) out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');

    return out;
}
} // namespace

namespace {
// Memory buffer struct for stb callback
struct JpegMemory {
    std::vector<uint8_t> data;
};

// stb callback: append to vector

// NOLINTNEXTLINE
void write_jpeg_callback(void* context, void* data, int size) {
    auto* buf = static_cast<JpegMemory*>(context);
    auto* bytes = static_cast<uint8_t*>(data);
    buf->data.insert(buf->data.end(), bytes, bytes + size);
}
} // namespace

// TODO: black bars on the top and right in headless mode!
Constants::Event General::dumpView() const {
    std::function<void()> const callback = [&]() -> void {
        Data::JSON view;

        // Get current window/render target size
        auto const window = domain.getSdlWindow();
        auto const renderer = domain.getSdlRenderer();
        int width, height;
        if (window) {
            // Normal windowed mode
            SDL_GetWindowSize(window, &width, &height);
        } else {
            // Headless mode - get renderer output size
            SDL_GetCurrentRenderOutputSize(renderer, &width, &height);
        }

        // Create surface to capture pixels
        SDL_Rect const fullScreenRect = {0, 0, width, height};

        // Read pixels into an SDL_Surface
        SDL_Surface* surface = SDL_RenderReadPixels(renderer, &fullScreenRect);
        if (!surface) {
            view.set("type", "error");
            view.set("message", SDL_GetError());
            domain.capture.log.println(view.serialize());
            return;
        }
        auto const pitch = static_cast<size_t>(surface->pitch);
        uint8_t const* pixels = static_cast<uint8_t*>(surface->pixels);

        // Using stb_image to convert to jpeg
        JpegMemory jpegBuffer;
        int constexpr jpegQuality = 80; // 0-100
        stbi_write_jpg_to_func(write_jpeg_callback, &jpegBuffer,
           width,
           height,
           4,       // channels (RGBA)
           pixels,  // pixel data from SDL_Surface
           jpegQuality
        );

        // Convert data to Base64
        std::string const encoded = base64_encode(jpegBuffer.data.data(), jpegBuffer.data.size());
        SDL_DestroySurface(surface);

        // Set values
        view.set("type","frame");
        view.set("width", width);
        view.set("height", height);
        view.set("format", "jpeg");
        view.set("pitch", pitch);
        view.set("encoding", "base64");
        view.set("data", encoded);

        // Instead of logging this info to the usual capture, we send it directly to cout.
        // Otherwise, this will clog up the domain viewer and make rendering super slow
        //domain.capture.log.println(view.serialize("", Data::RjDirectAccess::SerializationType::compact));
        std::cout << view.serialize("", Data::RjDirectAccess::SerializationType::compact) << std::endl;
    };
    domain.addPostRenderCallback(callback);
    return Constants::Event::Success;
}

Constants::Event General::selectedObjectGet(int const argc, char** argv){
    if (argc != 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    // Supports only uint32_t ids
    size_t const idx = std::stoul(argv[1]);
    if (auto objAndScope = domain.getObjectFromIndex(idx); objAndScope.has_value()) {
        auto [obj, scope] = objAndScope.value();
        selectedRenderObject = obj;
        selectedRenderObjectData = scope;
        return Constants::Event::Success;
    }
    selectedRenderObject = nullptr;
    domain.capture.warning.println("No RenderObject with ID ", idx, " found. Selection cleared.");
    return Constants::Event::Warning;
}

Constants::Event General::selectedObjectParse(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const command = Utility::StringHandler::recombineArgs(args.subspan(1));
    if (selectedRenderObject == nullptr || selectedRenderObjectData == nullptr) {
        domain.capture.warning.println("No RenderObject selected! Use selectedObjectGet <id> to select a valid object.");
        return Constants::Event::Warning;
    }

    Interaction::Context objectCtx = {
        {
            .self = *selectedRenderObject,
            .other = *selectedRenderObject,
            .global = ctx.global
        }
    };
    Interaction::ContextScope objectCtxScope = {
        {
            .self = *selectedRenderObjectData,
            .other = *selectedRenderObjectData,
            .global = ctxScope.global
        }
    };
    return selectedRenderObject->parseStr(std::string(__FUNCTION__) + " " + command, objectCtx, objectCtxScope);
}

} // namespace Nebulite::Module::Domain::Renderer
