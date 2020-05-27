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
set(MSVC_WARNINGS "${MSVC_WARNINGS} /W4")
set(CLANG_GCC_WARNINGS "${CLANG_GCC_WARNINGS} -Wall -Wextra -Wpedantic -Wno-c++98-compat")
if(CI)
    # fail CI builds on warnings
    set(MSVC_WARNINGS "${MSVC_WARNINGS} /WX")
    set(CLANG_GCC_WARNINGS "${CLANG_GCC_WARNINGS} -Werror")
endif()

#--------------------------------------------------------
# Compiler command line
#--------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_GCC_WARNINGS}")
    if(MSVC)
        # enables code navigation on errors in CLion with clang-cl
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /diagnostics:classic")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU.*")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_GCC_WARNINGS}")
elseif(MSVC) # last, because MSVC is defined in clang-cl
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_WARNINGS}")
else()
    message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif()
message(STATUS "CMAKE_CXX_FLAGS = '${CMAKE_CXX_FLAGS}'")

#--------------------------------------------------------
# Libraries
#--------------------------------------------------------

hunter_add_package(SDL2)
find_package(SDL2 CONFIG REQUIRED)
set(APP_LINKER_LIBS SDL2::SDL2)


#--------------------------------------------------------
# Outpout
#--------------------------------------------------------

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output)
add_executable(${CMAKE_PROJECT_NAME} src/main.cpp src/main.h)

if(MSVC)
    # This warnings informs us that the linker cannot find debug info pour external librairies; we don't care
    set(LINKER_FLAGS ${LINKER_FLAGS} -ignore:4099)
endif()

# use the non-debug version of SDL2 in RelWithDebInfo and MinSizeRel build types
set_target_properties(SDL2::SDL2 SDL2::SDL2main PROPERTIES
    MAP_IMPORTED_CONFIG_MINSIZEREL Release
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
)

target_link_libraries(${CMAKE_PROJECT_NAME} ${APP_LINKER_LIBS} ${LINKER_FLAGS})
