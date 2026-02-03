# Set minimum required version for CMake
cmake_minimum_required(VERSION 3.25)

# Define the project name
project(IncrementalGame VERSION 1.0)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify source files first
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# Create the executable target
add_executable(IncrementalGame ${SRC_FILES})
target_include_directories(IncrementalGame PRIVATE "include")

# --- Platform and Compiler-Specific Logic ---

# MSVC and MSYS
if(MSVC OR MSYS)
    # PDCurses-specific compiler definitions
    target_compile_definitions(IncrementalGame PRIVATE PDCURSES_WCS PDC_WIDE)

    # MSVC doesn't support -fno-trapping-math
    if(NOT MSVC)
        target_compile_definitions(IncrementalGame PRIVATE NO_TRAPPING_MATH)
        target_compile_options(IncrementalGame PRIVATE "-fno-trapping-math")
    endif()
    
    # Find the include and library paths for PDCurses
    find_path(PDCURSES_INCLUDE_DIR curses.h)
    
    if(PDCURSES_INCLUDE_DIR)
        target_include_directories(IncrementalGame PRIVATE ${PDCURSES_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "Could not find curses.h. Please set CMAKE_PREFIX_PATH to your PDCurses installation.")
    endif()

    find_library(PDCURSES_LIBRARIES NAMES pdcurses pdcurses.lib)
    if(NOT PDCURSES_LIBRARIES)
        message(FATAL_ERROR "Could not find PDCurses library. Please set CMAKE_PREFIX_PATH.")
    endif()

    if(MSVC)
        # MSVC linker and compiler options
        target_compile_options(IncrementalGame PRIVATE "/W2" "/WX" "/EHsc" "/utf-8")
        set(CMAKE_CXX_FLAGS_RELEASE "/O2")
        set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
        target_link_libraries(IncrementalGame PRIVATE ${PDCURSES_LIBRARIES})
        
    elseif(MSYS)
        # MSYS/MinGW linker and compiler options
        target_compile_options(IncrementalGame PRIVATE "-Wall" "-Wextra" "-Werror" "-march=native")
        set(CMAKE_CXX_FLAGS_RELEASE "-O3")
        set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
        target_link_libraries(IncrementalGame PRIVATE "-static" ${PDCURSES_LIBRARIES} "-lstdc++exp")
    endif()

# Unix (Linux/macOS)
elseif(UNIX)
    # Compiler flags and definitions
    target_compile_options(IncrementalGame PRIVATE "-Wall" "-Wextra" "-Werror" "-march=native" "-fno-trapping-math")
    target_compile_definitions(IncrementalGame PRIVATE NO_TRAPPING_MATH)

    # Find and link ncurses
    find_library(NCURSESW_LIBRARIES NAMES ncursesw)
    if(NOT NCURSESW_LIBRARIES)
        message(FATAL_ERROR "Could not find ncursesw library.")
    endif()
    target_link_libraries(IncrementalGame PRIVATE ${NCURSESW_LIBRARIES})

    # Set build-specific flags for Unix
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O2 -fsanitize=address,undefined")
endif()

# Set the output directories for the different build types
set_target_properties(IncrementalGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "bin"
)
