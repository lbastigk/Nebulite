//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

//------------------------------------------
namespace Nebulite::Constants {

// Warning

Event StandardCapture::Warning::Renderer::snapshotFailed(Utility::Io::Capture& cap) {
    cap.warning.println("Renderer snapshot failed.");
    return Event::warning;
}

Event StandardCapture::Warning::Functional::functionCallInvalid(Utility::Io::Capture& cap) {
    cap.warning.println("Requested function call is invalid.");
    return Event::warning;
}

Event StandardCapture::Warning::Functional::invalidArgcArgvParsing(Utility::Io::Capture& cap) {
    cap.warning.println("Invalid argc/argv parsing.");
    return Event::warning;
}

Event StandardCapture::Warning::Functional::tooManyArgs(Utility::Io::Capture& cap) {
    cap.warning.println("Too Many Arguments in function call");
    return Event::warning;
}

Event StandardCapture::Warning::Functional::tooFewArgs(Utility::Io::Capture& cap) {
    cap.warning.println("Too Few Arguments in function call");
    return Event::warning;
}

Event StandardCapture::Warning::Functional::unknownArg(Utility::Io::Capture& cap) {
    cap.warning.println("Unknown Argument");
    return Event::warning;
}

Event StandardCapture::Warning::Functional::invalidArgument(Utility::Io::Capture& cap) {
    cap.warning.println("Invalid Argument");
    return Event::warning;
}

Event StandardCapture::Warning::Functional::featureNotImplemented(Utility::Io::Capture& cap) {
    cap.warning.println("Requested feature of functioncall is not implemented");
    return Event::warning;
}

Event StandardCapture::Warning::Functional::bindingCollision(Utility::Io::Capture& cap) {
    cap.warning.println("Binding Collision: Failed to bind a function, category or variable.");
    return Event::warning;
}

Event StandardCapture::Warning::Functional::bindingCollisionExpected(Utility::Io::Capture& cap) {
    cap.warning.println("Binding Collision Expected: No collision occurred when one was expected.");
    return Event::warning;
}

// Error

Event StandardCapture::Error::Sdl::initFailed(Utility::Io::Capture& cap) {
    cap.error.println("SDL Renderer could not be initialized. This may cause rendering issues.");
    return Event::error;
}
Event StandardCapture::Error::Sdl::settingTargetFailed(Utility::Io::Capture& cap) {
    cap.error.println("SDL Renderer target could not be set.");
    return Event::error;
}

Event StandardCapture::Error::Renderer::notInitialized(Utility::Io::Capture& cap) {
    cap.error.println("Renderer not initialized.");
    return Event::error;
}

Event StandardCapture::Error::Texture::notFound(Utility::Io::Capture& cap) {
    cap.error.println("Texture not found.");
    return Event::error;
}

Event StandardCapture::Error::Texture::copyFailed(Utility::Io::Capture& cap) {
    cap.error.println("Texture copy failed.");
    return Event::error;
}

Event StandardCapture::Error::Texture::colorUnsupported(Utility::Io::Capture& cap) {
    cap.error.println("Texture color format unsupported.");
    return Event::error;
}

Event StandardCapture::Error::Texture::lockFailed(Utility::Io::Capture& cap) {
    cap.error.println("Texture lock failed.");
    return Event::error;
}

Event StandardCapture::Error::Texture::queryFailed(Utility::Io::Capture& cap) {
    cap.error.println("Texture query failed.");
    return Event::error;
}

Event StandardCapture::Error::Texture::modificationFailed(Utility::Io::Capture& cap) {
    cap.error.println("Texture modification failed.");
    return Event::error;
}

Event StandardCapture::Error::Texture::invalid(Utility::Io::Capture& cap) {
    cap.error.println("Texture is invalid.");
    return Event::error;
}

Event StandardCapture::Error::Audio::deviceInitFailed(Utility::Io::Capture& cap) {
    cap.error.println("Audio device could not be initialized.");
    return Event::error;
}

Event StandardCapture::Error::Functional::functionNotImplemented(Utility::Io::Capture& cap) {
    cap.error.println("Requested function not implemented.");
    return Event::error;
}

Event StandardCapture::Error::File::invalidFile(Utility::Io::Capture& cap) {
    cap.error.println("Requested file is invalid.");
    return Event::error;
}

Event StandardCapture::Error::File::couldNotWriteFile(Utility::Io::Capture& cap) {
    cap.error.println("Couldn't write to file.");
    return Event::error;
}

Event StandardCapture::Error::Ruleset::parsingFailed(Utility::Io::Capture& cap) {
    cap.error.println("Ruleset parsing failed.");
    return Event::error;
}

} // namespace Nebulite::Constants
