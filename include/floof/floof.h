#ifndef FLOOF_H
#define FLOOF_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque context -- holds the audio engine and all internal state. */
typedef struct floof_context floof_context;

/*
 * Initialise the audio engine.
 * On success *ctx is set to a valid context and 0 is returned.
 * On failure *ctx is set to NULL and -1 is returned.
 */
int floof_init(floof_context** ctx);

/*
 * Shut down the audio engine and free all resources.
 * Passing NULL is safe and does nothing.
 */
void floof_shutdown(floof_context* ctx);

/* Return the number of embedded sounds baked into the library. */
size_t floof_sound_count(void);

/*
 * Return the name of the embedded sound at `index`.
 * Returns NULL if the index is out of range.
 */
const char* floof_sound_name(size_t index);

/*
 * Play an embedded sound by name (fire-and-forget).
 * Returns 0 on success, -1 if the sound was not found or playback failed.
 */
int floof_play(floof_context* ctx, const char* name);

/*
 * Play a random embedded sound.
 * Returns 0 on success, -1 if no sounds are available or playback failed.
 */
int floof_play_random(floof_context* ctx);

#ifdef __cplusplus
}
#endif

#endif /* FLOOF_H */
