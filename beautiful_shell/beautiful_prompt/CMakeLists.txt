cmake_minimum_required(VERSION 3.15)
project(beautiful_prompt CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    libgit2
    GIT_REPOSITORY https://github.com/libgit2/libgit2.git
    GIT_TAG        v1.9.4
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_CLAR OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(USE_SSH OFF CACHE BOOL "" FORCE)
set(USE_HTTPS OFF CACHE BOOL "" FORCE)
set(USE_BUNDLED_ZLIB ON CACHE BOOL "" FORCE)
set(USE_NTLMCLIENT OFF CACHE BOOL "" FORCE)
set(BUILD_CLI OFF CACHE BOOL "" FORCE)
set(REGEX_BACKEND "builtin" CACHE STRING "" FORCE)

FetchContent_Populate(libgit2)

if(NOT EXISTS "${libgit2_BINARY_DIR}/libgit2.a" AND NOT EXISTS "${libgit2_BINARY_DIR}/git2.a")
    message(STATUS "Pre-building libgit2 statically...")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -S ${libgit2_SOURCE_DIR} -B ${libgit2_BINARY_DIR}
        -DBUILD_SHARED_LIBS=OFF 
        -DBUILD_CLAR=OFF 
        -DBUILD_TESTS=OFF
        -DREGEX_BACKEND=builtin
        -DUSE_SSH=OFF 
        -DUSE_HTTPS=OFF 
        -DUSE_BUNDLED_ZLIB=ON
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${libgit2_BINARY_DIR} --config Release
    )
endif()

add_executable(beautiful_prompt main.cpp)

target_include_directories(beautiful_prompt PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${libgit2_SOURCE_DIR}/include
)

# 6. Линкуем физический файл статического архива напрямую
target_compile_definitions(beautiful_prompt PRIVATE HAS_LIBGIT2)

if(APPLE)
    target_link_libraries(beautiful_prompt PRIVATE 
        "${libgit2_BINARY_DIR}/libgit2.a"
        "-framework CoreFoundation" 
        "-framework IOKit"
        iconv
    )
else()
    target_link_libraries(beautiful_prompt PRIVATE 
        "${libgit2_BINARY_DIR}/libgit2.a"
        pthread dl
    )
endif()