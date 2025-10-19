# CMake Toolchains

This directory contains CMake toolchain files for cross-platform builds.

## Available Toolchains

### `windows-mingw.cmake`
**Windows builds using MinGW/GCC**
- **On Windows**: Uses native MinGW-w64 or MSYS2 tools (`gcc.exe`, `g++.exe`)
- **On Linux**: Uses MinGW cross-compilation tools (`x86_64-w64-mingw32-gcc`)
- **Features**: Static linking, no DLL dependencies, works across platforms

### `macos-cross.cmake`
**macOS builds using Clang**
- **On macOS**: Uses native Xcode/Command Line Tools (`clang`, `clang++`)
- **On Linux**: Uses osxcross for cross-compilation (requires setup)
- **Features**: Auto-detects architecture (Intel/Apple Silicon), works across platforms

## Usage

These toolchains are automatically used by CMake presets:

```bash
# Windows builds (works on Windows and Linux)
cmake --preset windows-debug
cmake --preset windows-release

# macOS builds (works on macOS and Linux with osxcross)
cmake --preset macos-debug
cmake --preset macos-release
```

## Requirements

### For Windows cross-compilation on Linux:
```bash
sudo dnf install mingw64-gcc mingw64-gcc-c++
```

### For macOS cross-compilation on Linux:
1. Install osxcross following the instructions in the main documentation
2. Ensure `OSXCROSS_ROOT` environment variable is set or osxcross is in standard location

### For native builds:
- **Windows**: Install MinGW-w64 or MSYS2
- **macOS**: Install Xcode or Command Line Tools