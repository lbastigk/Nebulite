# Windows MinGW toolchain for both native Windows builds and cross-compilation
# - On Windows: Uses native MinGW-w64 or MSYS2 tools
# - On Linux: Uses MinGW cross-compilation tools
#
# This toolchain provides Windows builds using GCC/MinGW across platforms

# Target platform
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 1)

# Detect if we're running on native Windows
cmake_host_system_information(RESULT HOST_SYSTEM QUERY OS_NAME)
if(HOST_SYSTEM MATCHES "Windows")
    set(NATIVE_WINDOWS TRUE)
    message(STATUS "Detected native Windows build environment")
else()
    set(NATIVE_WINDOWS FALSE)
    message(STATUS "Detected cross-compilation build environment (Linux -> Windows)")
endif()

if(NATIVE_WINDOWS)
    # Native Windows build - use system MinGW tools
    message(STATUS "Using native Windows MinGW tools")
    
    # Let CMake find MinGW compilers in PATH (MSYS2, MinGW-w64, etc.)
    # CMAKE_C_COMPILER and CMAKE_CXX_COMPILER will be auto-detected
    # Common locations: C:/msys64/mingw64/bin/, C:/MinGW/bin/, etc.
    
    # Try to find MinGW compilers explicitly if needed
    find_program(CMAKE_C_COMPILER NAMES gcc.exe mingw32-gcc.exe x86_64-w64-mingw32-gcc.exe)
    find_program(CMAKE_CXX_COMPILER NAMES g++.exe mingw32-g++.exe x86_64-w64-mingw32-g++.exe)
    
    if(NOT CMAKE_C_COMPILER OR NOT CMAKE_CXX_COMPILER)
        message(WARNING "MinGW compilers not found in PATH")
        message(WARNING "Please install MinGW-w64 or MSYS2 and add to PATH")
    endif()
    
else()
    # Cross-compilation setup (Linux -> Windows)
    message(STATUS "Using MinGW cross-compilation tools")
    
    # Cross-compilation compilers
    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    
    # Path for MinGW includes and libs
    set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
    
    # Adjust for cross-compilation
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

# C++ Standard and threads
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthreads")

# Prefer pthreads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

# Linking
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -mwindows")

if(NATIVE_WINDOWS)
    message(STATUS "Native Windows MinGW toolchain loaded")
else()
    message(STATUS "Windows cross-compilation toolchain loaded")
endif()