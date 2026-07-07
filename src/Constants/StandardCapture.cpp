//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Constants {

// Warning

Event StandardCapture::Warning::Renderer::snapshotFailed(Utility::IO::Capture& cap) {
    cap.warning.println("Renderer snapshot failed.");
    return Event::Warning;
}

Event StandardCapture::Warning::Functional::functionCallInvalid(Utility::IO::Capture& cap) {
    cap.warning.println("Requested function call is invalid.");
    return Event::Warning;
}

Event StandardCapture::Warning::Functional::invalidArgcArgvParsing(Utility::IO::Capture& cap) {
    cap.warning.println("argc/argv parsing error.");
    return Event::Warning;
}

Event StandardCapture::Warning::Functional::tooManyArgs(Utility::IO::Capture& cap) {
    cap.warning.println("Too Many Arguments in function call");
    return Event::Warning;
}

Event StandardCapture::Warning::Functional::tooFewArgs(Utility::IO::Capture& cap) {
    cap.warning.println("Too Few Arguments in function call");
    return Event::Warning;
}

Event StandardCapture::Warning::Functional::unknownArg(Utility::IO::Capture& cap) {
    cap.warning.println("Unknown Argument Error");
    return Event::Warning;
}

Event StandardCapture::Warning::Functional::featureNotImplemented(Utility::IO::Capture& cap) {
    cap.warning.println("Requested feature of functioncall is not implemented");
    return Event::Warning;
}

Event StandardCapture::Warning::Functional::bindingCollision(Utility::IO::Capture& cap) {
    cap.warning.println("Binding Collision: Failed to bind a function, category or variable.");
    return Event::Warning;
}

Event StandardCapture::Warning::Functional::bindingCollisionExpected(Utility::IO::Capture& cap) {
    cap.warning.println("Binding Collision Expected: No collision occurred when one was expected.");
    return Event::Warning;
}

// Error

Event StandardCapture::Error::SDL::initFailed(Utility::IO::Capture& cap) {
    cap.error.println("SDL Renderer could not be initialized. This may cause rendering issues.");
    return Event::Error;
}
Event StandardCapture::Error::SDL::settingTargetFailed(Utility::IO::Capture& cap) {
    cap.error.println("SDL Renderer target could not be set.");
    return Event::Error;
}

Event StandardCapture::Error::Renderer::notInitialized(Utility::IO::Capture& cap) {
    cap.error.println("Renderer not initialized.");
    return Event::Error;
}

Event StandardCapture::Error::Texture::notFound(Utility::IO::Capture& cap) {
    cap.error.println("Texture not found.");
    return Event::Error;
}

Event StandardCapture::Error::Texture::copyFailed(Utility::IO::Capture& cap) {
    cap.error.println("Texture copy failed.");
    return Event::Error;
}

Event StandardCapture::Error::Texture::colorUnsupported(Utility::IO::Capture& cap) {
    cap.error.println("Texture color format unsupported.");
    return Event::Error;
}

Event StandardCapture::Error::Texture::lockFailed(Utility::IO::Capture& cap) {
    cap.error.println("Texture lock failed.");
    return Event::Error;
}

Event StandardCapture::Error::Texture::queryFailed(Utility::IO::Capture& cap) {
    cap.error.println("Texture query failed.");
    return Event::Error;
}

Event StandardCapture::Error::Texture::modificationFailed(Utility::IO::Capture& cap) {
    cap.error.println("Texture modification failed.");
    return Event::Error;
}

Event StandardCapture::Error::Texture::invalid(Utility::IO::Capture& cap) {
    cap.error.println("Texture is invalid.");
    return Event::Error;
}

Event StandardCapture::Error::Audio::deviceInitFailed(Utility::IO::Capture& cap) {
    cap.error.println("Audio device could not be initialized.");
    return Event::Error;
}

Event StandardCapture::Error::Functional::functionNotImplemented(Utility::IO::Capture& cap) {
    cap.error.println("Requested function not implemented.");
    return Event::Error;
}

Event StandardCapture::Error::File::invalidFile(Utility::IO::Capture& cap) {
    cap.error.println("Requested file is invalid.");
    return Event::Error;
}

Event StandardCapture::Error::File::couldNotWriteFile(Utility::IO::Capture& cap) {
    cap.error.println("Couldn't write to file.");
    return Event::Error;
}

Event StandardCapture::Error::Ruleset::parsingFailed(Utility::IO::Capture& cap) {
    cap.error.println("Ruleset parsing failed.");
    return Event::Error;
}

} // namespace Nebulite::Constants
