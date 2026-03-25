#include "Nebulite.hpp"
#include "DomainModule/Renderer/General.hpp"
#include "Core/Renderer.hpp"       // The domain
#include "Interaction/Invoke.hpp"  // Invoke for parsing expressions
#include "Core/RenderObject.hpp"   // RenderObject for Renderer

namespace Nebulite::DomainModule::Renderer {

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

// NOLINTNEXTLINE
Constants::Event General::envLoad(int const argc, char** argv) const {
    if (argc > 1) {
        domain.deserialize(argv[1]);
        return Constants::Event::Success;
    }
    // no name provided, load empty env
    domain.deserialize("{}");
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

        // Append to renderer
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
    if (argc == 1) {
        // No link provided, use default
        if (!domain.snapshot("./Resources/Snapshots/snapshot.png")) {
            return Constants::StandardCapture::Warning::Renderer::snapshotFailed(domain.capture);
        }
        return Constants::Event::Success;
    }
    if (argc == 2) {
        // Link provided
        if (!domain.snapshot(argv[1])) {
            return Constants::StandardCapture::Warning::Renderer::snapshotFailed(domain.capture);
        }
        return Constants::Event::Success;
    }
    return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
}

Constants::Event General::selectedObject_get(int const argc, char** argv){
    if (argc != 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    // Supports only uint32_t ids
    size_t const idx = std::stoul(argv[1]);
    if (Core::RenderObject* obj = domain.getObjectFromIndex(idx); obj) {
        selectedRenderObject = obj;
        return Constants::Event::Success;
    }
    selectedRenderObject = nullptr;
    domain.capture.warning.println("No RenderObject with ID ", idx, " found. Selection cleared.");
    return Constants::Event::Warning;
}

Constants::Event General::selectedObject_Parse(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const command = Utility::StringHandler::recombineArgs(args.subspan(1));
    if (selectedRenderObject == nullptr) {
        domain.capture.warning.println("No RenderObject selected! Use selectedObject_get <id> to select a valid object.");
        return Constants::Event::Warning;
    }

    return selectedRenderObject->parseStr(std::string(__FUNCTION__) + " " + command);
}

} // namespace Nebulite::DomainModule::Renderer
