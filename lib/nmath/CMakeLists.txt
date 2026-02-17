set(MICROTEX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/micro_tex")

file(GLOB_RECURSE MICROTEX_ALL_SOURCES "${MICROTEX_DIR}/src/*.cpp")

file(GLOB_RECURSE MICROTEX_PLATFORM_SOURCES "${MICROTEX_DIR}/src/platform/*.cpp")
list(REMOVE_ITEM MICROTEX_ALL_SOURCES ${MICROTEX_PLATFORM_SOURCES})

add_library(microtex STATIC ${MICROTEX_ALL_SOURCES})

target_compile_features(microtex PUBLIC cxx_std_17)

target_include_directories(microtex SYSTEM PUBLIC "${MICROTEX_DIR}/src")

set_target_properties(microtex PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)
if(lto_supported)
    set_target_properties(microtex PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()