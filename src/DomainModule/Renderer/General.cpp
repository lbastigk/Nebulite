#include "Nebulite.hpp"
#include "DomainModule/Renderer/General.hpp"
#include "Core/Renderer.hpp"       // The domain
#include "Interaction/Invoke.hpp"  // Invoke for parsing expressions
#include "Core/RenderObject.hpp"   // RenderObject for Renderer

namespace Nebulite::DomainModule::Renderer {

//------------------------------------------
// Update
Constants::Error General::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Constants::Error General::envLoad(int argc, char** argv) {
    if (argc > 1) {
        domain->deserialize(argv[1]);
        return Constants::ErrorTable::NONE();
    }
    // no name provided, load empty env
    domain->deserialize("{}");
    return Constants::ErrorTable::NONE();
}

Constants::Error General::envDeload() {
    domain->purgeObjects();
    domain->purgeTextures();
    return Constants::ErrorTable::NONE();
}

Constants::Error General::spawn(int argc, char** argv) {
    if (argc > 1) {
        // Using all args, allowing for whitespaces in the link and in the following functioncalls:
        // e.g.: spawn Planets/sun.jsonc|set text.str This is a sun
        std::string linkOrObject = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);

        // Check if the file exists
        if (std::string const link = Utility::StringHandler::untilSpecialChar(linkOrObject, '|'); !Utility::FileManagement::fileExists(link)) {
            // Check in standard directories
            static std::vector<std::string> standardDirectories = {
                "./Resources/Renderobjects/",
                "./Resources/RenderObjects/"
            };

            // Check all standard directories for the file
            bool found = false;
            for (auto const& prefix : standardDirectories) {
                if (std::string const testLink = prefix + link; Utility::FileManagement::fileExists(testLink)) {
                    linkOrObject.insert(0, prefix);
                    found = true;
                    break;
                }
            }

            // Not found in standard directories either
            if (!found) {
                return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
            }
        }

        // Create object with link to globalspace
        auto* ro = new Core::RenderObject;
        ro->deserialize(linkOrObject);

        // Append to renderer
        // Renderer manages the RenderObjects lifetime
        domain->append(ro);
    } else {
        Nebulite::cerr() << "No renderobject name provided!" << Nebulite::endl;
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::setResolution(int argc, char** argv) {
    int w = 1000;
    int h = 1000;
    uint16_t scalar = 1;
    if (argc > 1) {
        w = std::stoi(argv[1]);
    }
    if (argc > 2) {
        h = std::stoi(argv[2]);
    }
    if (argc > 3) {
        if (int const signedScalar = std::stoi(argv[3]); signedScalar > 0) {
            scalar = static_cast<uint16_t>(signedScalar);
        } else {
            scalar = 1;
        }
    }
    domain->changeWindowSize(w, h, scalar);
    return Constants::ErrorTable::NONE();
}

Constants::Error General::setFPS(int argc, char** argv) {
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
    domain->setTargetFPS(fps);
    return Constants::ErrorTable::NONE();
}

Constants::Error General::showFPS(int argc, char** argv) {
    if (argc < 2) {
        domain->toggleFps(true);
    } else {
        if (!strcmp(argv[1], "on")) {
            domain->toggleFps(true);
        } else if (!strcmp(argv[1], "off")) {
            domain->toggleFps(false);
        } else {
            // unknown arg
            return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::cam_move(int argc, char** argv) {
    if (argc < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    int const dx = std::stoi(argv[1]);
    int const dy = std::stoi(argv[2]);
    domain->moveCam(dx, dy);
    return Constants::ErrorTable::NONE();
}

Constants::Error General::cam_set(int argc, char** argv) {
    if (argc == 3) {
        int const x = std::stoi(argv[1]);
        int const y = std::stoi(argv[2]);
        domain->setCam(x, y);
        return Constants::ErrorTable::NONE();
    }
    if (argc == 4) {
        if (!strcmp(argv[3], "c")) {
            int const x = std::stoi(argv[1]);
            int const y = std::stoi(argv[2]);
            domain->setCam(x, y, true);
            return Constants::ErrorTable::NONE();
        }
        // unknown arg
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    if (argc > 4) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
}

Constants::Error General::snapshot(int argc, char** argv) {
    if (argc == 1) {
        // No link provided, use default
        if (!domain->snapshot("./Resources/Snapshots/snapshot.png")) {
            return Constants::ErrorTable::RENDERER::CRITICAL_RENDERER_SNAPSHOT_FAILED();
        }
        return Constants::ErrorTable::NONE();
    }
    if (argc == 2) {
        // Link provided
        if (!domain->snapshot(argv[1])) {
            return Constants::ErrorTable::RENDERER::CRITICAL_RENDERER_SNAPSHOT_FAILED();
        }
        return Constants::ErrorTable::NONE();
    }
    return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
}

Constants::Error General::beep() {
    // Beep function for debugging, from SDL
    domain->beep();
    return Constants::ErrorTable::NONE();
}

Constants::Error General::selectedObject_get(int argc, char** argv) {
    if (argc != 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    // Supports only uint32_t ids
    uint32_t const id = static_cast<uint32_t>(std::stoul(argv[1]));
    if (Core::RenderObject* obj = domain->getObjectFromId(id); obj) {
        selectedRenderObject = obj;
        return Constants::ErrorTable::NONE();
    }
    selectedRenderObject = nullptr;
    return Constants::ErrorTable::addError("No RenderObject with the specified ID found.", Constants::Error::NON_CRITICAL);
}

Constants::Error General::selectedObject_Parse(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const command = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    if (selectedRenderObject == nullptr) {
        return Constants::ErrorTable::addError("No RenderObject selected! Use selectedObject_get <id> to select a valid object.", Constants::Error::NON_CRITICAL);
    }

    return selectedRenderObject->parseStr(std::string(__FUNCTION__) + " " + command);
}

} // namespace Nebulite::DomainModule::Renderer
