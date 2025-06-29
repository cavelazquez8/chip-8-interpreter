# FindSDL2.cmake - Cross-platform SDL2 finder
# This module provides cross-platform SDL2 detection for the CHIP-8 interpreter

# Try pkg-config first (Linux/Unix)
if(NOT WIN32)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(PC_SDL2 QUIET sdl2)
    endif()
endif()

# Find SDL2 headers
find_path(SDL2_INCLUDE_DIR
    NAMES SDL.h
    HINTS
        ${PC_SDL2_INCLUDEDIR}
        ${PC_SDL2_INCLUDE_DIRS}
        ${SDL2_ROOT}/include
        $ENV{SDL2_ROOT}/include
        $ENV{SDL2DIR}/include
    PATH_SUFFIXES SDL2
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
        # Windows specific paths
        "C:/SDL2/include"
        "C:/SDL2-**/include"
        # macOS specific paths
        /Library/Frameworks/SDL2.framework/Headers
        /System/Library/Frameworks/SDL2.framework/Headers
)

# Find SDL2 library
find_library(SDL2_LIBRARY
    NAMES SDL2 SDL2-static
    HINTS
        ${PC_SDL2_LIBDIR}
        ${PC_SDL2_LIBRARY_DIRS}
        ${SDL2_ROOT}/lib
        $ENV{SDL2_ROOT}/lib
        $ENV{SDL2DIR}/lib
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
        # Windows specific paths
        "C:/SDL2/lib/x64"
        "C:/SDL2/lib/x86"
        "C:/SDL2-**/lib/x64"
        "C:/SDL2-**/lib/x86"
        # macOS specific paths
        /Library/Frameworks/SDL2.framework
        /System/Library/Frameworks/SDL2.framework
)

# Find SDL2main library (required on Windows)
if(WIN32)
    find_library(SDL2_MAIN_LIBRARY
        NAMES SDL2main
        HINTS
            ${PC_SDL2_LIBDIR}
            ${PC_SDL2_LIBRARY_DIRS}
            ${SDL2_ROOT}/lib
            $ENV{SDL2_ROOT}/lib
            $ENV{SDL2DIR}/lib
        PATHS
            "C:/SDL2/lib/x64"
            "C:/SDL2/lib/x86"
            "C:/SDL2-**/lib/x64"
            "C:/SDL2-**/lib/x86"
    )
endif()

# Handle version
if(PC_SDL2_VERSION)
    set(SDL2_VERSION ${PC_SDL2_VERSION})
elseif(SDL2_INCLUDE_DIR AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_version.h")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_LINES
        REGEX "^#define[ \t]+SDL_[A-Z]+_VERSION[ \t]+[0-9]+")
    string(REGEX REPLACE ".*#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+).*" "\\1"
        SDL2_VERSION_MAJOR "${SDL2_VERSION_LINES}")
    string(REGEX REPLACE ".*#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+).*" "\\1"
        SDL2_VERSION_MINOR "${SDL2_VERSION_LINES}")
    string(REGEX REPLACE ".*#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+).*" "\\1"
        SDL2_VERSION_PATCH "${SDL2_VERSION_LINES}")
    set(SDL2_VERSION "${SDL2_VERSION_MAJOR}.${SDL2_VERSION_MINOR}.${SDL2_VERSION_PATCH}")
endif()

# Set up imported targets
if(SDL2_LIBRARY AND SDL2_INCLUDE_DIR)
    if(NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 UNKNOWN IMPORTED)
        set_target_properties(SDL2::SDL2 PROPERTIES
            IMPORTED_LOCATION "${SDL2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}")
        
        # Platform-specific link requirements
        if(WIN32)
            set_target_properties(SDL2::SDL2 PROPERTIES
                INTERFACE_LINK_LIBRARIES "winmm;imm32;ole32;oleaut32;version;uuid;advapi32;setupapi;shell32")
        elseif(APPLE)
            set_target_properties(SDL2::SDL2 PROPERTIES
                INTERFACE_LINK_LIBRARIES "-framework Cocoa;-framework IOKit;-framework CoreVideo;-framework CoreAudio;-framework AudioToolbox;-framework ForceFeedback;-framework Metal")
        endif()
    endif()
    
    if(WIN32 AND SDL2_MAIN_LIBRARY AND NOT TARGET SDL2::SDL2main)
        add_library(SDL2::SDL2main UNKNOWN IMPORTED)
        set_target_properties(SDL2::SDL2main PROPERTIES
            IMPORTED_LOCATION "${SDL2_MAIN_LIBRARY}")
    endif()
endif()

# Standard CMake package handling
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR
    VERSION_VAR SDL2_VERSION)

# Set output variables for compatibility
if(SDL2_FOUND)
    set(SDL2_LIBRARIES ${SDL2_LIBRARY})
    set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
    
    if(WIN32 AND SDL2_MAIN_LIBRARY)
        list(APPEND SDL2_LIBRARIES ${SDL2_MAIN_LIBRARY})
    endif()
endif()

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY SDL2_MAIN_LIBRARY)