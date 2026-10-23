cmake_minimum_required(VERSION 3.15)
project(beautiful_shell CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

# Fully static, dependency-free executable for portable release builds.
# Not supported on Apple (no static libSystem); off by default elsewhere.
# -static + glibc breaks NSS/getpwuid (username lookup fails at runtime).
# Linux release builds use musl via Alpine, where fully static linking is safe.
option(BSHELL_STATIC "Statically link the final executable (portable release builds)" OFF)

include(FetchContent)
include(ExternalProject)
include(CheckCXXCompilerFlag)

# Portable release baseline: never tune to the build host's CPU (no -march=native).
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
    set(_bshell_arch_candidate "-march=x86-64-v2")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(_bshell_arch_candidate "-march=armv8-a")
endif()

set(BSHELL_ARCH_FLAGS "")
if(_bshell_arch_candidate)
    check_cxx_compiler_flag("${_bshell_arch_candidate}" BSHELL_ARCH_FLAG_OK)
    if(BSHELL_ARCH_FLAG_OK)
        set(BSHELL_ARCH_FLAGS "${_bshell_arch_candidate}")
    endif()
endif()
message(STATUS "beautiful_shell release arch baseline: '${BSHELL_ARCH_FLAGS}'")

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

FetchContent_GetProperties(libgit2_src)
if(NOT libgit2_src_POPULATED)
    FetchContent_Populate(
        libgit2_src
        GIT_REPOSITORY https://github.com/libgit2/libgit2.git
        GIT_TAG v1.9.4
        SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/libgit2_src-src"
        BINARY_DIR "${CMAKE_BINARY_DIR}/_deps/libgit2_src-build"
        SUBBUILD_DIR "${CMAKE_BINARY_DIR}/_deps/libgit2_src-subbuild"
    )
endif()

set(LIBGIT2_FILENAME "libgit2.a")
set(LIBGIT2_BINARY_DIR "${CMAKE_BINARY_DIR}/libgit2_build")
set(LIBGIT2_STATIC_LIBRARY "${LIBGIT2_BINARY_DIR}/${LIBGIT2_FILENAME}")

# Keep libgit2's objects matching the main executable's minimum macOS version.
set(_bshell_libgit2_extra_args "")
if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    list(APPEND _bshell_libgit2_extra_args "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

ExternalProject_Add(
    libgit2_project
    SOURCE_DIR "${libgit2_src_SOURCE_DIR}"
    BINARY_DIR "${LIBGIT2_BINARY_DIR}"
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${LIBGIT2_STATIC_LIBRARY}"
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_C_FLAGS=${BSHELL_ARCH_FLAGS}
    -DBUILD_SHARED_LIBS=OFF
    -DBUILD_TESTS=OFF
    -DBUILD_CLI=OFF
    -DUSE_SSH=OFF
    -DUSE_HTTPS=OFF
    -DUSE_ICONV=OFF
    -DREGEX_BACKEND=builtin
    -DUSE_SHA1=CollisionDetection
    -DUSE_BUNDLED_ZLIB=ON
    ${_bshell_libgit2_extra_args}
)

add_library(git2_static STATIC IMPORTED GLOBAL)
set_target_properties(git2_static PROPERTIES
    IMPORTED_LOCATION "${LIBGIT2_STATIC_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${libgit2_src_SOURCE_DIR}/include"
)

add_dependencies(git2_static libgit2_project)
add_executable(beautiful_shell main.cpp)

if(BSHELL_ARCH_FLAGS)
    target_compile_options(beautiful_shell PRIVATE ${BSHELL_ARCH_FLAGS})
endif()

if(MSVC)
    target_compile_options(beautiful_shell PRIVATE /W4)
else()
    target_compile_options(beautiful_shell PRIVATE -Wall -Wextra)
endif()

target_include_directories(beautiful_shell PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(beautiful_shell PRIVATE
    git2_static
    tomlplusplus::tomlplusplus
)

find_package(Threads REQUIRED)
target_link_libraries(beautiful_shell PRIVATE Threads::Threads)

if(APPLE)
    target_link_libraries(beautiful_shell PRIVATE
        "-framework CoreFoundation"
        "-framework IOKit"
        "-framework Security"
)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(beautiful_shell PRIVATE dl)
endif()

# Portable release builds: link the C/C++ runtime and all deps statically so the
# resulting binary has no external shared-library requirements. Apple does not
# support fully static executables, so this is skipped there. Do not enable
# this on glibc hosts (NSS/getpwuid will fail); use musl instead.
if(BSHELL_STATIC AND NOT APPLE)
    target_link_options(beautiful_shell PRIVATE -static)
endif()
