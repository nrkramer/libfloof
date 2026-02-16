# libfloof

A library with the sole purpose of helping applications play cute cat noises.

## Features

- Cross-platform audio playback via [miniaudio](https://miniaud.io/)
- Sounds are baked directly into the library binary -- no external files needed at runtime
- Pure C API for easy integration and cross-language FFI bindings
- Drop audio files into a folder and CMake handles the rest

## Prerequisites

- CMake 3.21+
- A C++17 compiler
- [vcpkg](https://vcpkg.io/) with the `VCPKG_ROOT` environment variable set

## Building

```bash
cmake --preset default
cmake --build build
```

For a release build:

```bash
cmake --preset release
cmake --build build
```

## Adding Sounds

Drop audio files into the `sounds/` directory and rebuild. CMake will automatically pick them up, convert them to embedded C arrays, and register them in the library.

Supported formats: `.wav`, `.flac`, `.aiff`, `.mp3`, `.ogg`

Compressed formats (mp3, ogg) are recommended over lossless (wav, aiff) for smaller binary sizes since the raw bytes are embedded directly.

The filename (without extension) becomes the sound name used in the API. For example, `sounds/purr.wav` is played with:

```c
floof_play(ctx, "purr");
```

## Running the Example

After building:

```bash
./build/examples/meow
```

This lists all embedded sounds and plays one at random.

## API

```c
#include <floof/floof.h>

floof_context* ctx = NULL;
floof_init(&ctx);

floof_sound_count();          // number of embedded sounds
floof_sound_name(0);          // name of sound at index
floof_play(ctx, "meow");      // play a sound by name
floof_play_random(ctx);       // play a random sound

floof_shutdown(ctx);
```

| Function | Description |
|---|---|
| `floof_init(&ctx)` | Initialise the audio engine |
| `floof_shutdown(ctx)` | Shut down and free all resources |
| `floof_sound_count()` | Return the number of embedded sounds |
| `floof_sound_name(i)` | Return the name of a sound by index |
| `floof_play(ctx, name)` | Play a sound by name (fire-and-forget) |
| `floof_play_random(ctx)` | Play a random embedded sound |

## License

Public domain ([Unlicense](https://unlicense.org)). See [LICENSE](LICENSE).
