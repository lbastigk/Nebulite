# macOS cross-compilation toolchain for both native macOS builds and cross-compilation
# - On macOS: Uses native tools (Xcode/Command Line Tools)
# - On Linux: Uses osxcross for cross-compilation
#
# This toolchain provides macOS builds using Clang across platforms
# Work in progress, not fully tested yet!

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_VERSION 22.0) # macOS Ventura or later

# Detect if we're running on native macOS
cmake_host_system_information(RESULT HOST_SYSTEM QUERY OS_NAME)
if(HOST_SYSTEM MATCHES "macOS")
    set(NATIVE_MACOS TRUE)
    message(STATUS "Detected native macOS build environment")
else()
    set(NATIVE_MACOS FALSE)
    message(STATUS "Detected cross-compilation build environment")
endif()

# Handle cross-compilation setup (Linux -> macOS)
if(NOT NATIVE_MACOS)
    # Detect osxcross installation
    if(NOT DEFINED ENV{OSXCROSS_ROOT})
        if(EXISTS "/opt/osxcross")
            set(OSXCROSS_ROOT "/opt/osxcross")
        elseif(EXISTS "$ENV{HOME}/osxcross")
            set(OSXCROSS_ROOT "$ENV{HOME}/osxcross")
        else()
            message(WARNING "osxcross not found at common locations (/opt/osxcross, ~/osxcross)")
            message(WARNING "Please install osxcross or set OSXCROSS_ROOT environment variable")
            message(WARNING "macOS cross-compilation will not work without osxcross")
            # Set a placeholder path to continue configuration (will fail at compile time)
            set(OSXCROSS_ROOT "/path/to/osxcross")
            set(OSXCROSS_MISSING TRUE)
        endif()
    else()
        set(OSXCROSS_ROOT "$ENV{OSXCROSS_ROOT}")
    endif()
endif()

# Set target architecture (x86_64 or arm64)
if(NOT DEFINED OSX_ARCH)
    if(NATIVE_MACOS)
        # On native macOS, detect the architecture automatically
        cmake_host_system_information(RESULT HOST_ARCH QUERY OS_PLATFORM)
        if(HOST_ARCH MATCHES "arm64")
            set(OSX_ARCH "arm64") # Apple Silicon (M1/M2/M3)
        else()
            set(OSX_ARCH "x86_64") # Intel Macs
        endif()
    else()
        set(OSX_ARCH "x86_64") # Default to Intel for cross-compilation
    endif()
endif()

# Set macOS version
if(NOT DEFINED OSX_VERSION)
    set(OSX_VERSION "13.0") # macOS Ventura
endif()

set(CMAKE_OSX_ARCHITECTURES ${OSX_ARCH})
set(CMAKE_OSX_DEPLOYMENT_TARGET ${OSX_VERSION})

if(NATIVE_MACOS)
    # Native macOS build - use system tools
    message(STATUS "Using native macOS build tools")
    
    # Let CMake find the system compilers (Xcode/Command Line Tools)
    # CMAKE_C_COMPILER and CMAKE_CXX_COMPILER will be auto-detected
    
    # Set deployment target and architecture
    set(CMAKE_OSX_DEPLOYMENT_TARGET ${OSX_VERSION})
    set(CMAKE_OSX_ARCHITECTURES ${OSX_ARCH})
    
else()
    # Cross-compilation setup (Linux -> macOS)
    message(STATUS "Using osxcross for cross-compilation")
    
    # Configure cross-compilation tools
    set(CMAKE_C_COMPILER "${OSXCROSS_ROOT}/bin/${OSX_ARCH}-apple-darwin22-clang")
    set(CMAKE_CXX_COMPILER "${OSXCROSS_ROOT}/bin/${OSX_ARCH}-apple-darwin22-clang++")
    set(CMAKE_AR "${OSXCROSS_ROOT}/bin/${OSX_ARCH}-apple-darwin22-ar")
    set(CMAKE_RANLIB "${OSXCROSS_ROOT}/bin/${OSX_ARCH}-apple-darwin22-ranlib")
    set(CMAKE_LINKER "${OSXCROSS_ROOT}/bin/${OSX_ARCH}-apple-darwin22-ld")
    
    # Set sysroot
    set(CMAKE_SYSROOT "${OSXCROSS_ROOT}/SDK/MacOSX${OSX_VERSION}.sdk")
    
    # Search paths for libraries and includes
    set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
    
    # Compiler and linker flags
    set(CMAKE_C_FLAGS_INIT "-target ${OSX_ARCH}-apple-macos${OSX_VERSION}")
    set(CMAKE_CXX_FLAGS_INIT "-target ${OSX_ARCH}-apple-macos${OSX_VERSION}")
endif()

if(NATIVE_MACOS)
    message(STATUS "Native macOS toolchain loaded")
    message(STATUS "Target: ${OSX_ARCH}-apple-macos${OSX_VERSION}")
else()
    message(STATUS "macOS cross-compilation toolchain loaded")
    message(STATUS "Target: ${OSX_ARCH}-apple-macos${OSX_VERSION}")
    message(STATUS "osxcross root: ${OSXCROSS_ROOT}")
endif()