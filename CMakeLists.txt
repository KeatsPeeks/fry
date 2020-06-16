cmake_minimum_required(VERSION 3.15)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

#--------------------------------------------------------
# Hunter package manager
#--------------------------------------------------------
include("HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.254.tar.gz"
    SHA1 "017e688b9848c1f695303e8cfe9f35d40e154112"
)

project(fry)

#--------------------------------------------------------
# Minimum C++ version required
#--------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------------------------------------------------
# Versionning (build version is provided by Appveyor)
#--------------------------------------------------------
if(BUILD_VERSION)
    add_definitions(-DBUILD_VERSION="${BUILD_VERSION}")
endif()

#--------------------------------------------------------
# Static analysis
#--------------------------------------------------------
if(ENABLE_STATIC_ANALYSIS)
    find_program(CLANGTIDY NAMES clang-tidy clang-tidy-10 clang-tidy-9 clang-tidy-8 clang-tidy-7 clang-tidy-6.0)
    if(CLANGTIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
    else()
        message(FATAL_ERROR "clang-tidy not found")
    endif()

    find_program(CPPCHECK NAMES cppcheck)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK ${CPPCHECK} --enable=all --inconclusive --inline-suppr --language=c++)
    else()
        message(FATAL_ERROR "cppcheck not found")
    endif()
endif()

#--------------------------------------------------------
# Warnings
#--------------------------------------------------------
set(MSVC_WARNINGS "${MSVC_WARNINGS} /W4 /wd4068")
set(CLANG_GCC_WARNINGS "${CLANG_GCC_WARNINGS} -Wall -Wextra -Wpedantic -Wno-c++98-compat -Wno-unknown-pragmas")
set(CLANG_WARNINGS "${CLANG_GCC_WARNINGS}")
set(GCC_WARNINGS "${CLANG_GCC_WARNINGS}")
if(CI)
    # fail CI builds on warnings
    set(MSVC_WARNINGS "${MSVC_WARNINGS} /WX")
    set(CLANG_WARNINGS "${CLANG_GCC_WARNINGS} -Werror")
    # set(GCC_WARNINGS "${CLANG_GCC_WARNINGS} -Werror") not for obnoxious GCC because nuklear's warnings are unavoidable
endif()

#--------------------------------------------------------
# Compiler command line
#--------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_WARNINGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.10")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.10")        
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU.*")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_WARNINGS} -DENABLE_PMR")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_WARNINGS} /DENABLE_PMR /DNOMINMAX")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4068")
else()
    message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif()
message(STATUS "CMAKE_CXX_FLAGS = '${CMAKE_CXX_FLAGS}'")
message(STATUS "CMAKE_C_FLAGS = '${CMAKE_C_FLAGS}'")

#--------------------------------------------------------
# Libraries
#--------------------------------------------------------

hunter_add_package(SDL2)
find_package(SDL2 CONFIG REQUIRED)
hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)
hunter_add_package(SDL_ttf)
find_package(SDL_ttf CONFIG REQUIRED)
set(APP_LINKER_LIBS SDL2::SDL2 SDL2::SDL2main spdlog::spdlog SDL_ttf::SDL_ttf)


#--------------------------------------------------------
# Outpout
#--------------------------------------------------------

if(MSVC)
    # WIN32: changes the subsytem from console to Windows to prevent extra console windows creation
    set(WIN32_EXE "WIN32")

    # LNK4099: informs us that the linker cannot find debug info pour external librairies; we don't care
    # LNK4075: "ignoring '/INCREMENTAL' due to '/OPT:REF' specification"
    # /OPT:Ref: always eliminates functions and data that are never referenced, even with debug info
    # /ENTRY:mainCRTStartup: required for /subsystem:windows
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ignore:4099 -ignore:4075 /OPT:Ref")

endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output)
file(GLOB SDL2_GFX_FILES deps/SDL2_gfx/*.c)
file(GLOB NUKLEAR_FILES deps/nuklear/*.c)
set(APP_FILES
        src/clock.h
        src/colors.h
        src/game.cpp
        src/game.h
        src/gui.cpp
        src/gui.h
        src/main.cpp
        src/nuklear_sdl.cpp
        src/nuklear_sdl.h
        src/pattern.cpp
        src/pattern.h
        src/primitives.h
        src/sdl_wrappers.h
        src/simulation.cpp
        src/simulation.h
        src/version.h
)
add_executable(${CMAKE_PROJECT_NAME} ${WIN32_EXE} ${SDL2_GFX_FILES} ${NUKLEAR_FILES} ${APP_FILES})

# use the non-debug versions for RelWithDebInfo and MinSizeRel build types
set_target_properties(SDL2::SDL2 SDL2::SDL2main spdlog::spdlog fmt::fmt SDL_ttf::SDL_ttf ZLIB::zlib PNG::png BZip2::bz2 freetype::freetype PROPERTIES
    MAP_IMPORTED_CONFIG_MINSIZEREL Release
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
)

target_link_libraries(${CMAKE_PROJECT_NAME} ${APP_LINKER_LIBS})

file(COPY assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
