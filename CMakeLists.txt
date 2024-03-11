cmake_minimum_required(VERSION 3.13)
project(BatScript)

# Actually want C++20, but <format> isn't available there yet on MSVC
# Instead, use C++23 which maps to latest in VS
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

# Enable multithreaded builds
if (MSVC)
    add_compile_options(/MP)
    add_compile_options("/Zc:preprocessor")
    add_compile_options(/wd4251)
else()
    set(CMAKE_COLOR_DIAGNOSTICS ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -mcpu=apple-m1 -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -D_DEBUG -g")
endif()

include_directories(inc)

add_subdirectory(src)
