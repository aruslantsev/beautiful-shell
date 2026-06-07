cmake_minimum_required(VERSION 3.15)
project(beautiful_shell CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(BUILD_CLAR OFF CACHE BOOL "Disable libgit2 tests" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "Disable tests" FORCE)
set(USE_SSH OFF CACHE BOOL "Disable SSH support" FORCE)
set(USE_HTTPS OFF CACHE BOOL "Disable HTTPS support" FORCE)
set(USE_BUNDLED_ZLIB ON CACHE BOOL "Use bundled zlib" FORCE)
set(USE_NTLMCLIENT OFF CACHE BOOL "Disable NTLM" FORCE)
set(BUILD_CLI OFF CACHE BOOL "Disable CLI tools for libgit2" FORCE)
set(REGEX_BACKEND "builtin" CACHE STRING "Use builtin regex backend" FORCE)

FetchContent_Declare(
    libgit2
    GIT_REPOSITORY https://github.com/libgit2/libgit2.git
    GIT_TAG        v1.9.4
)
FetchContent_MakeAvailable(libgit2)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

add_executable(beautiful_shell main.cpp)

target_include_directories(beautiful_shell PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${libgit2_SOURCE_DIR}/include
)

target_link_libraries(beautiful_shell PRIVATE 
    git2
    tomlplusplus::tomlplusplus
)
