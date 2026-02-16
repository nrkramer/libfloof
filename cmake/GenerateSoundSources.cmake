# GenerateSoundSources.cmake
# Converts audio files in SOUNDS_DIR into C source files with embedded byte arrays,
# plus a registry that indexes all embedded sounds.
#
# Usage: cmake -DSOUNDS_DIR=<path> -DOUTPUT_DIR=<path> -P GenerateSoundSources.cmake

if(NOT DEFINED SOUNDS_DIR OR NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "SOUNDS_DIR and OUTPUT_DIR must be defined")
endif()

file(GLOB SOUND_FILES
    "${SOUNDS_DIR}/*.wav"
    "${SOUNDS_DIR}/*.flac"
    "${SOUNDS_DIR}/*.aiff"
)
list(SORT SOUND_FILES)

set(ALL_NAMES "")
set(ALL_C_NAMES "")

foreach(sound_file ${SOUND_FILES})
    get_filename_component(filename "${sound_file}" NAME)
    get_filename_component(name "${sound_file}" NAME_WE)
    string(MAKE_C_IDENTIFIER "${name}" c_name)

    list(APPEND ALL_NAMES "${name}")
    list(APPEND ALL_C_NAMES "${c_name}")

    # Read file as a continuous hex string and get its size
    file(READ "${sound_file}" hex_content HEX)
    file(SIZE "${sound_file}" file_size)

    # Insert a newline every 32 hex chars (16 bytes) for readable output
    string(REGEX REPLACE "([0-9a-f]{32})" "\\1\n" hex_content "${hex_content}")

    # Convert each hex byte pair to 0xNN, format
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," hex_content "${hex_content}")

    # Indent continuation lines
    string(REPLACE "\n" "\n    " hex_content "${hex_content}")

    file(WRITE "${OUTPUT_DIR}/sound_${c_name}.c"
"/* Auto-generated from ${filename} -- do not edit */
#include <stddef.h>

const unsigned char floof_sound_${c_name}_data[] = {
    ${hex_content}
};

const size_t floof_sound_${c_name}_size = ${file_size};
")

    message(STATUS "Embedded sound: ${filename} (${file_size} bytes)")
endforeach()

# ---------------------------------------------------------------------------
# sound_registry.h
# ---------------------------------------------------------------------------
file(WRITE "${OUTPUT_DIR}/sound_registry.h"
"/* Auto-generated sound registry -- do not edit */
#ifndef FLOOF_SOUND_REGISTRY_H
#define FLOOF_SOUND_REGISTRY_H

#include <stddef.h>

#ifdef __cplusplus
extern \"C\" {
#endif

typedef struct {
    const char*          name;
    const unsigned char* data;
    size_t               size;
} floof_embedded_sound_t;

extern const floof_embedded_sound_t floof_embedded_sounds[];
extern const size_t                 floof_embedded_sound_count;

#ifdef __cplusplus
}
#endif

#endif /* FLOOF_SOUND_REGISTRY_H */
")

# ---------------------------------------------------------------------------
# sound_registry.c
# ---------------------------------------------------------------------------
list(LENGTH ALL_NAMES sound_count)

set(src "/* Auto-generated sound registry -- do not edit */\n")
string(APPEND src "#include \"sound_registry.h\"\n\n")

# Extern declarations for each sound's data and size arrays
if(sound_count GREATER 0)
    math(EXPR last "${sound_count} - 1")
    foreach(i RANGE 0 ${last})
        list(GET ALL_C_NAMES ${i} c_name)
        string(APPEND src "extern const unsigned char floof_sound_${c_name}_data[];\n")
        string(APPEND src "extern const size_t        floof_sound_${c_name}_size;\n")
    endforeach()
    string(APPEND src "\n")
endif()

# Sound table
string(APPEND src "const floof_embedded_sound_t floof_embedded_sounds[] = {\n")
if(sound_count GREATER 0)
    foreach(i RANGE 0 ${last})
        list(GET ALL_NAMES ${i} name)
        list(GET ALL_C_NAMES ${i} c_name)
        string(APPEND src "    { \"${name}\", floof_sound_${c_name}_data, floof_sound_${c_name}_size },\n")
    endforeach()
else()
    # C requires at least one element; this sentinel is never accessed
    string(APPEND src "    { 0, 0, 0 }\n")
endif()
string(APPEND src "};\n\n")

string(APPEND src "const size_t floof_embedded_sound_count = ${sound_count};\n")

file(WRITE "${OUTPUT_DIR}/sound_registry.c" "${src}")

message(STATUS "Sound registry generated with ${sound_count} sound(s)")
