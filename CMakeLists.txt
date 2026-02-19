cmake_minimum_required(VERSION 4.0)
project(nyx LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_link_options("-fuse-ld=lld")
endif()

include(CTest)

add_library(skia INTERFACE)

target_include_directories(skia INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia")

file(GLOB SKIA_ALL_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia/out/Release/*.a")

target_link_libraries(skia INTERFACE
        -Wl,--allow-multiple-definition
        -Wl,--whole-archive
        -Wl,--start-group
        ${SKIA_ALL_LIBS}
        -Wl,--end-group
        -Wl,--no-whole-archive
        pthread
        dl
        fontconfig
        freetype
        GL
        icuuc
        icui18n
        icudata
)

add_library(stb_imagewrite INTERFACE)
target_include_directories(stb_imagewrite INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb_imagewrite")

add_subdirectory(lib/nlayout)

file(COPY "${CMAKE_SOURCE_DIR}/assets/fonts" DESTINATION "${CMAKE_BINARY_DIR}")

target_compile_definitions(nlayout_test PRIVATE
        ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

file(COPY "${CMAKE_SOURCE_DIR}/third_party/micro_tex/res" DESTINATION "${CMAKE_BINARY_DIR}")

add_subdirectory(lib/nmath)
