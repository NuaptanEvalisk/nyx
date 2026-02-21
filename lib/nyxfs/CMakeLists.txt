add_library(nyxfs SHARED
        include/nyxfs/nyxfs.h
        include/nyxfs/nyxfs_types.h
        include/nyxfs/nyxfs_err_list.h
        src/nyxfs_mgmt.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(nyxfs PRIVATE
            -O3                 # Ultimate optimization level
            -march=native       # Tailor code to your specific CPU (AVX2/AVX-512)
            -fno-math-errno
            -fno-trapping-math
            -fassociative-math
            -freciprocal-math
            -fno-plt            # Avoid Procedure Linkage Table overhead for internal calls
            -fPIC
            -fno-exceptions
            -fno-stack-protector
            -fstrict-aliasing
            -fomit-frame-pointer
    )
endif()

target_link_options(nyxfs PRIVATE "-Wl,-z,defs")

target_include_directories(nyxfs PUBLIC include)
target_include_directories(nyxfs PRIVATE src)
target_include_directories(nyxfs PRIVATE tests)

find_package(Threads REQUIRED)
target_link_libraries(nyxfs PRIVATE Threads::Threads)

set_target_properties(nyxfs PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)

enable_testing()
