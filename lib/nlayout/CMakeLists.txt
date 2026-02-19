add_library(nlayout SHARED
        src/nlayout_mgmt.cpp
        include/nlayout/nlayout_types.h
        src/nlayout_internals.hpp
        src/nlayout_vm.cpp
        src/nlayout_extractors.cpp
        src/nlayout_hit_test.cpp
        include/nlayout/nlayout_err_list.h
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(nlayout PRIVATE
            -O3                 # Ultimate optimization level
            -march=native       # Tailor code to your specific CPU (AVX2/AVX-512)
            -fno-math-errno
            -fno-trapping-math
            -fassociative-math
            -freciprocal-math
            -fno-plt            # Avoid Procedure Linkage Table overhead for internal calls
            -fno-exceptions
            -fno-rtti
            -fno-stack-protector
            -fstrict-aliasing
            -fomit-frame-pointer
    )
endif()

target_link_options(nlayout PRIVATE "-Wl,-z,defs")

target_include_directories(nlayout PUBLIC include)
target_include_directories(nlayout PRIVATE src)
target_include_directories(nlayout PRIVATE tests)

# we are using Skia m146.
target_link_libraries(nlayout PRIVATE skia)

add_executable(nlayout_test tests/nlayout_test.c)
target_compile_options(nlayout_test PRIVATE -O3 -march=native)
target_link_libraries(nlayout_test PRIVATE nlayout)
target_link_libraries(nlayout_test PRIVATE stb_imagewrite)

enable_testing()
add_test(NAME nlayout_demo COMMAND nlayout_test)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

if(lto_supported)
    message(STATUS "IPO/LTO is supported. Enabling for nlayout and nlayout_test.")
    set_target_properties(nlayout PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_target_properties(nlayout_test PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO/LTO is not supported: ${lto_error}")
endif()