cmake_minimum_required(VERSION 3.16)
project(barcode_project CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data" CACHE PATH "Folder that holds Product.txt and the other data files")

enable_testing()

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

file(GLOB BARCODE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(FILTER BARCODE_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
add_library(barcode_lib STATIC ${BARCODE_SOURCES})
target_include_directories(barcode_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

if(EXISTS ${CMAKE_SOURCE_DIR}/src/main.cpp)
    add_executable(barcode ${CMAKE_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(barcode barcode_lib)
endif()

set(POS_CORE_SOURCES "")
foreach(layer common domain storage services scanning)
    file(GLOB_RECURSE LAYER_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/pos/${layer}/*.cpp)
    list(APPEND POS_CORE_SOURCES ${LAYER_SOURCES})
endforeach()
list(FILTER POS_CORE_SOURCES EXCLUDE REGEX "QtCameraSource")

if(POS_CORE_SOURCES)
    add_library(pos_core STATIC ${POS_CORE_SOURCES})
    target_include_directories(pos_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_compile_definitions(pos_core PUBLIC POS_DATA_DIR="${DATA_DIR}")
    target_link_libraries(pos_core PUBLIC barcode_lib)
else()
    add_library(pos_core INTERFACE)
    target_include_directories(pos_core INTERFACE ${CMAKE_SOURCE_DIR}/include)
    target_compile_definitions(pos_core INTERFACE POS_DATA_DIR="${DATA_DIR}")
    target_link_libraries(pos_core INTERFACE barcode_lib)
endif()

find_package(Qt6 COMPONENTS Widgets Multimedia QUIET)

if(Qt6_FOUND)
    file(GLOB_RECURSE POS_QT_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/pos/gui/*.cpp)
    file(GLOB_RECURSE POS_QT_HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/pos/gui/*.hpp)

    set(QT_CAMERA_CPP ${CMAKE_SOURCE_DIR}/src/pos/scanning/QtCameraSource.cpp)
    set(QT_CAMERA_HPP ${CMAKE_SOURCE_DIR}/include/pos/scanning/QtCameraSource.hpp)
    if(EXISTS ${QT_CAMERA_CPP})
        list(APPEND POS_QT_SOURCES ${QT_CAMERA_CPP})
    endif()
    if(EXISTS ${QT_CAMERA_HPP})
        list(APPEND POS_QT_HEADERS ${QT_CAMERA_HPP})
    endif()

    if(POS_QT_SOURCES)
        add_library(pos_qt STATIC ${POS_QT_SOURCES} ${POS_QT_HEADERS})
        target_link_libraries(pos_qt PUBLIC pos_core Qt6::Widgets Qt6::Multimedia)

        if(EXISTS ${CMAKE_SOURCE_DIR}/src/pos/main_pos.cpp)
            add_executable(pos_app ${CMAKE_SOURCE_DIR}/src/pos/main_pos.cpp)
            target_link_libraries(pos_app pos_qt)

            if(WIN32)
                find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
                if(WINDEPLOYQT)
                    add_custom_command(TARGET pos_app POST_BUILD
                        COMMAND "${WINDEPLOYQT}" "$<TARGET_FILE:pos_app>")
                endif()
            endif()
        endif()
    endif()
else()
    message(STATUS "Qt6 not found: pos_qt and pos_app are skipped")
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/src/pos/tools/generate_barcodes_main.cpp)
    add_executable(generate_barcodes
        ${CMAKE_SOURCE_DIR}/src/pos/tools/generate_barcodes_main.cpp
        ${CMAKE_SOURCE_DIR}/src/pos/tools/BarcodeAssetGenerator.cpp)
    target_link_libraries(generate_barcodes pos_core)
endif()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/test_*.cpp)
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} pos_core)
    target_compile_definitions(${test_name} PRIVATE SAMPLES_DIR="${CMAKE_SOURCE_DIR}/samples")
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
