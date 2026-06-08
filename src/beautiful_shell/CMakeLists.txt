 cmake_minimum_required(VERSION 3.15)
project(beautiful_shell CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(ExternalProject)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

FetchContent_Declare(
    libgit2_src
    GIT_REPOSITORY https://github.com/libgit2/libgit2.git
    GIT_TAG v1.9.4
)
FetchContent_GetProperties(libgit2_src)
if(NOT libgit2_src_POPULATED)
    FetchContent_Populate(libgit2_src)
endif()

set(LIBGIT2_FILENAME "libgit2.a")
set(LIBGIT2_BINARY_DIR "${CMAKE_BINARY_DIR}/libgit2_build")
set(LIBGIT2_STATIC_LIBRARY "${LIBGIT2_BINARY_DIR}/${LIBGIT2_FILENAME}")

ExternalProject_Add(
    libgit2_project
    SOURCE_DIR "${libgit2_src_SOURCE_DIR}"
    BINARY_DIR "${LIBGIT2_BINARY_DIR}"
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${LIBGIT2_STATIC_LIBRARY}"
    CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_SHARED_LIBS=OFF
    -DBUILD_CLAR=OFF
    -DBUILD_TESTS=OFF
    -DBUILD_CLI=OFF
    -DUSE_SSH=OFF
    -DUSE_HTTPS=OFF
    -DUSE_ICONV=OFF
    -DREGEX_BACKEND=builtin
    -DSHA1_BACKEND=collisiondetect
    -DUSE_BUNDLED_ZLIB=ON
)

add_library(git2_static STATIC IMPORTED GLOBAL)
set_target_properties(git2_static PROPERTIES
    IMPORTED_LOCATION "${LIBGIT2_STATIC_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${libgit2_src_SOURCE_DIR}/include"
)

add_dependencies(git2_static libgit2_project)
add_executable(beautiful_shell main.cpp)

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
