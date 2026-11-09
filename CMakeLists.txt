cmake_minimum_required(VERSION 3.12)
project(BarcodeProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------- Library: every .cpp in src/ except main.cpp ----------
# CONFIGURE_DEPENDS makes CMake notice new .cpp files automatically,
# so nobody has to edit this file when they add a module.
file(GLOB LIB_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

add_library(barcode_lib ${LIB_SOURCES})
# Headers live in include/barcode/ and are used as #include "barcode/xyz.hpp"
target_include_directories(barcode_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Warnings help catch mistakes early
if(MSVC)
    target_compile_options(barcode_lib PRIVATE /W4)
else()
    target_compile_options(barcode_lib PRIVATE -Wall -Wextra)
endif()

# ---------- Program: menu and command line (main.cpp) ----------
if(EXISTS ${CMAKE_SOURCE_DIR}/src/main.cpp)
    add_executable(barcode src/main.cpp)
    target_link_libraries(barcode barcode_lib)
endif()

# ---------- Tests (CTest) ----------
# A test is built only if its file exists, so the build works
# even before every member has pushed their tests.
enable_testing()

set(TEST_NAMES
    test_scanner       # Person 1
    test_ean13         # Person 2
    test_encoder       # Person 2
    test_validator     # Person 3
    test_image_io      # Person 3
    test_renderer      # Person 3
    test_preprocess    # Person 4
    test_decoder       # Person 4
    test_roundtrip     # Person 4
    test_samples)      # Person 5

foreach(t ${TEST_NAMES})
    if(EXISTS ${CMAKE_SOURCE_DIR}/tests/${t}.cpp)
        add_executable(${t} tests/${t}.cpp)
        target_link_libraries(${t} barcode_lib)
        # Lets tests find the sample images: SAMPLES_DIR "/samples"
        target_compile_definitions(${t} PRIVATE SAMPLES_DIR="${CMAKE_SOURCE_DIR}/samples")
        add_test(NAME ${t} COMMAND ${t})
    endif()
endforeach()
