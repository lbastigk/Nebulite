#include "Nebulite.hpp"
#include "DomainModule/Renderer/General.hpp"
#include "Core/Renderer.hpp"       // The domain
#include "Interaction/Invoke.hpp"  // Invoke for parsing expressions
#include "Core/RenderObject.hpp"   // RenderObject for Renderer

namespace Nebulite::DomainModule::Renderer {

std::string const General::cam_name = "cam";
std::string const General::cam_desc = R"(Renderer Camera Functions)";

std::string const General::selectedObject_name = "selected-object";
std::string const General::selectedObject_desc = R"(Functions to select and interact with a selected RenderObject)";

std::string const General::env_name = "env";
std::string const General::env_desc = R"(Environment management functions)";

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

// NOLINTNEXTLINE
Constants::Error General::env_load(int argc, char** argv) {
    if (argc > 1) {
        domain->deserialize(argv[1]);
        return Constants::ErrorTable::NONE();
    }
    // no name provided, load empty env
    domain->deserialize("{}");
    return Constants::ErrorTable::NONE();
}

std::string const General::env_load_name = "env load";
std::string const General::env_load_desc = R"(Load an environment/level from a json/jsonc file.

Usage: env load <path/to/file.jsonc>

If no argument is provided, an empty environment is loaded.
)";

// NOLINTNEXTLINE
Constants::Error General::env_deload(int argc, char** argv) {
    domain->purgeObjects();
    domain->purgeTextures();
    return Constants::ErrorTable::NONE();
}

std::string const General::env_deload_name = "env deload";
std::string const General::env_deload_desc = R"(Deload entire environment, leaving an empty renderer.

Usage: env deload
)";

// NOLINTNEXTLINE
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

std::string const General::spawn_name = "spawn";
std::string const General::spawn_desc = R"(Spawn a RenderObject from a json/jsonc file.

Usage: spawn <path/to/file.jsonc>

Supports lookups in standard resource directories:
- ./Resources/RenderObjects/
- ./Resources/Renderobjects/

Example: 'spawn Planets/sun.jsonc|set text.str This is a sun'
Looks for object 'sun.jsonc' in the standard directories
- './Planets/sun.jsonc'
- './Resources/RenderObjects/Planets/sun.jsonc'
- './Resources/Renderobjects/Planets/sun.jsonc'
and spawns the first found object.
)";

// NOLINTNEXTLINE
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

std::string const General::setResolution_name = "set-res";
std::string const General::setResolution_desc = R"(Set resolution of renderer.

Usage: set-res [Width] [Height] [Scale]

Defaults to 1000  for width if argument count < 1
Defaults to 1000  for height if argument count < 2
Defaults to 1     for scale if argument count < 3
)";

// NOLINTNEXTLINE
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

std::string const General::setFPS_name = "set-fps";
std::string const General::setFPS_desc = R"(Set FPS of renderer.

Usage: set-fps [fps]

Defaults to 60 fps if no argument is provided
)";

// NOLINTNEXTLINE
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

std::string const General::showFPS_name = "show-fps";
std::string const General::showFPS_desc = R"(Show FPS of renderer.

Usage: show-fps [on|off]

Defaults to on if no argument is provided
)";

// NOLINTNEXTLINE
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

std::string const General::cam_move_name = "cam move";
std::string const General::cam_move_desc = R"(Move camera by a given delta.

Usage: cam move <dx> <dy>

<dx> : Delta x to move camera by
<dy> : Delta y to move camera by
)";

// NOLINTNEXTLINE
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

std::string const General::cam_set_name = "cam set";
std::string const General::cam_set_desc = R"(Set camera to concrete position.

Usage: cam set <x> <y> [c]

<x> : X position to set camera to
<y> : Y position to set camera to
[c] : Optional. If provided, sets the camera's center to the given position.
)";

// NOLINTNEXTLINE
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

std::string const General::snapshot_name = "snapshot";
std::string const General::snapshot_desc = R"(Create a snapshot of the current renderer state.

Usage: snapshot [filename]

Defaults to "./Resources/Snapshots/snapshot.png" if no argument is provided
)";

// NOLINTNEXTLINE
Constants::Error General::beep(int argc, char** argv) {
    // Beep function for debugging, from SDL
    domain->beep();
    return Constants::ErrorTable::NONE();
}

std::string const General::beep_name = "beep";
std::string const General::beep_desc = R"(Make a beep noise.

Usage: beep
)";

// NOLINTNEXTLINE
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

std::string const General::selectedObject_get_name = "selected-object get";
std::string const General::selectedObject_get_desc = R"(Get a renderobject by its ID.

Usage: selected-object get <id>
)";

// NOLINTNEXTLINE
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

std::string const General::selectedObject_Parse_name = "selected-object parse";
std::string const General::selectedObject_Parse_desc = R"(Parse a command on the selected RenderObject.

Usage: selected-object parse <command>

Use 'selected-object get <id>' to select a RenderObject first.
Use 'selected-object parse help' to see available commands for the selected object.
)";

} // namespace Nebulite::DomainModule::Renderer
