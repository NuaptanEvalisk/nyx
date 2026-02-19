find_package(tinyxml2 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER REQUIRED poppler-cpp)

add_library(nmath SHARED
    src/nmath_internals.hpp
    src/nmath_mgmt.cpp
    include/nmath/nmath.h
    include/nmath/nmath_err_list.h
    include/nmath/nmath_types.h
        src/nmath_microtex_skia.cpp
        src/nmath_microtex.cpp
        src/nmath_lualatex_preload.cpp
        src/nmath_lualatex.cpp
        src/nmath_latex_spawn.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(nmath PRIVATE
            -O3                 # Ultimate optimization level
            -march=native       # Tailor code to your specific CPU (AVX2/AVX-512)
            -fno-math-errno
            -fno-trapping-math
            -fassociative-math
            -freciprocal-math
            -fno-plt            # Avoid Procedure Linkage Table overhead for internal calls
            -frtti
            -fPIC
            -fno-exceptions
            -fno-stack-protector
            -fstrict-aliasing
            -fomit-frame-pointer
    )
endif()

target_link_options(nmath PRIVATE "-Wl,-z,defs")

target_include_directories(nmath PUBLIC include)
target_include_directories(nmath PRIVATE src)
target_include_directories(nmath PRIVATE tests)

target_include_directories(nmath PRIVATE ${POPPLER_INCLUDE_DIRS})

set(MICROTEX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/micro_tex")

file(GLOB_RECURSE MICROTEX_ALL_SOURCES "${MICROTEX_DIR}/src/*.cpp")

file(GLOB_RECURSE MICROTEX_PLATFORM_SOURCES "${MICROTEX_DIR}/src/platform/*.cpp")
list(REMOVE_ITEM MICROTEX_ALL_SOURCES ${MICROTEX_PLATFORM_SOURCES})

add_library(microtex STATIC ${MICROTEX_ALL_SOURCES})

target_compile_features(microtex PUBLIC cxx_std_17)

target_compile_options(microtex PUBLIC -fPIC)

target_include_directories(microtex SYSTEM PUBLIC "${MICROTEX_DIR}/src")

set_target_properties(microtex PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)
if(lto_supported)
    set_target_properties(microtex PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# we are using Skia m146.
target_link_libraries(nmath PRIVATE skia)
target_link_libraries(nmath PRIVATE microtex tinyxml2::tinyxml2 ${POPPLER_LIBRARIES})

find_package(Threads REQUIRED)
target_link_libraries(nmath PRIVATE Threads::Threads m)

set_target_properties(nmath PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)

set(NMATH_LATEX_ASSETS
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/latex/nmath_latex_fmt.tex"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/latex/nmath_latex_head.tex"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/latex/nmath_latex_tail.tex"
)

file(COPY ${NMATH_LATEX_ASSETS} DESTINATION "${CMAKE_BINARY_DIR}/assets/latex")

add_executable(nmath_test_microtex tests/nmath_test_microtex.c)

target_compile_options(nmath_test_microtex PRIVATE -O3 -march=native)
target_link_libraries(nmath_test_microtex PRIVATE nmath)
target_link_libraries(nmath_test_microtex PRIVATE stb_imagewrite)

add_executable(nmath_test_lualatex tests/nmath_test_lualatex.c)

target_compile_options(nmath_test_lualatex PRIVATE -O3 -march=native)
target_link_libraries(nmath_test_lualatex PRIVATE nmath)
target_link_libraries(nmath_test_lualatex PRIVATE stb_imagewrite)

enable_testing()
add_test(NAME nmath_demo_microtex COMMAND nmath_test_microtex)
add_test(NAME nmath_demo_lualatex COMMAND nmath_test_lualatex)
