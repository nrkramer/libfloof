cmake_minimum_required(VERSION 3.21)
project(floof VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- Dependencies ---
find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")

# --- Embedded Sound Generation ---
# Drop .wav, .flac, or .aiff files into sounds/ and they will be
# automatically embedded into the library at build time.
# FLAC is recommended over WAV for smaller binary sizes.
set(SOUNDS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sounds")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_DIR}")

file(GLOB SOUND_FILES CONFIGURE_DEPENDS
    "${SOUNDS_DIR}/*.wav"
    "${SOUNDS_DIR}/*.flac"
    "${SOUNDS_DIR}/*.aiff"
)

# Predict generated file names so CMake knows what the custom command produces
set(GENERATED_SOURCES "${GENERATED_DIR}/sound_registry.c")
set(GENERATED_HEADERS "${GENERATED_DIR}/sound_registry.h")

foreach(sound_file ${SOUND_FILES})
    get_filename_component(name "${sound_file}" NAME_WE)
    string(MAKE_C_IDENTIFIER "${name}" c_name)
    list(APPEND GENERATED_SOURCES "${GENERATED_DIR}/sound_${c_name}.c")
endforeach()

add_custom_command(
    OUTPUT ${GENERATED_SOURCES} ${GENERATED_HEADERS}
    COMMAND ${CMAKE_COMMAND}
        -DSOUNDS_DIR=${SOUNDS_DIR}
        -DOUTPUT_DIR=${GENERATED_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateSoundSources.cmake
    DEPENDS ${SOUND_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateSoundSources.cmake
    COMMENT "Embedding sound files into C sources"
    VERBATIM
)

# --- Library Target ---
add_library(floof
    src/floof.cpp
    ${GENERATED_SOURCES}
    ${GENERATED_HEADERS}
)

target_include_directories(floof
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${GENERATED_DIR}
        ${MINIAUDIO_INCLUDE_DIRS}
)

# Platform-specific audio backend dependencies
if(APPLE)
    target_link_libraries(floof PRIVATE
        "-framework CoreFoundation"
        "-framework CoreAudio"
        "-framework AudioToolbox"
    )
elseif(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(floof PRIVATE Threads::Threads ${CMAKE_DL_LIBS} m)
endif()

# --- Examples ---
option(FLOOF_BUILD_EXAMPLES "Build example programs" ON)
if(FLOOF_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
