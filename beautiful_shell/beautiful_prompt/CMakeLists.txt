cmake_minimum_required(VERSION 3.15)
project(beautiful_prompt CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. Force static libgit2 build
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_CLAR OFF CACHE BOOL "" FORCE)
set(USE_SSH OFF CACHE BOOL "" FORCE)
set(USE_HTTPS OFF CACHE BOOL "" FORCE)
set(USE_BUNDLED_ZLIB ON CACHE BOOL "" FORCE)
set(USE_NTLMCLIENT OFF CACHE BOOL "" FORCE)
set(BUILD_CLI OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    libgit2
    GIT_REPOSITORY https://github.com/libgit2/libgit2.git
    GIT_TAG        v1.9.4
)

FetchContent_MakeAvailable(libgit2)
set(HAS_LIBGIT2 TRUE)

# 2. Explicitly define file name
add_executable(beautiful_prompt main.cpp)

target_include_directories(beautiful_prompt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 3. Link libgit2 to the project
if(HAS_LIBGIT2)
    target_compile_definitions(beautiful_prompt PRIVATE HAS_LIBGIT2)
    target_link_libraries(beautiful_prompt PRIVATE git2)
endif()

# 4. MacOS dependencies
if(APPLE)
    target_link_libraries(beautiful_prompt PRIVATE "-framework CoreFoundation" "-framework IOKit")
endif()