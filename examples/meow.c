#include <floof/floof.h>
#include <stdio.h>

int main(void)
{
    floof_context* ctx = NULL;

    if (floof_init(&ctx) != 0) {
        fprintf(stderr, "Failed to initialise floof\n");
        return 1;
    }

    size_t count = floof_sound_count();
    printf("libfloof has %zu embedded sound(s):\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("  - %s\n", floof_sound_name(i));
    }

    if (count > 0) {
        printf("Playing a random cat sound...\n");
        if (floof_play_random(ctx) != 0) {
            fprintf(stderr, "Playback failed!\n");
        } else {
            printf("Press Enter to exit.\n");
            getchar();
        }
    } else {
        printf("No sounds embedded -- drop .wav/.flac files into sounds/\n");
    }

    floof_shutdown(ctx);
    return 0;
}
