/* platform/audio_sdl.c — salida de audio por SDL3 (M4/T-042).
 *
 * Sólo se compila en el target `make port-sdl` (necesita SDL3). Abre el
 * dispositivo de playback en mono S16 a 48 kHz y lo alimenta desde el anillo que
 * llena `platform/audio.c`. Se usa `SDL_OpenAudioDeviceStream` con un callback
 * get: SDL pide datos y el callback drena el anillo.
 *
 * Sin dispositivo (headless) la emulación sigue corriendo: solo no hay drenaje,
 * así que el anillo se descarta y queda la medida de RMS/energía.
 */
#include <SDL3/SDL.h>
#include <stdint.h>

#include "platform.h"

static SDL_AudioStream *g_stream;

static void SDLCALL audio_get(void *userdata, SDL_AudioStream *stream,
                              int additional_amount, int total_amount) {
    int16_t buf[1024];
    (void)userdata;
    (void)total_amount;

    while (additional_amount > 0) {
        int want = additional_amount / (int)sizeof(int16_t);
        if (want > (int)(sizeof buf / sizeof buf[0]))
            want = (int)(sizeof buf / sizeof buf[0]);
        int n = platform_audio_pull(buf, want);
        if (n <= 0)
            break;
        SDL_PutAudioStreamData(stream, buf, n * (int)sizeof(int16_t));
        additional_amount -= n * (int)sizeof(int16_t);
    }
}

void platform_audio_sdl_open(void) {
    if (!platform_audio_enabled())
        return;

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("SDL_InitSubSystem(AUDIO): %s", SDL_GetError());
        return;
    }

    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.format   = SDL_AUDIO_S16;
    spec.channels = 1;
    spec.freq     = 48000;

    g_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                         &spec, audio_get, NULL);
    if (!g_stream) {
        SDL_Log("SDL_OpenAudioDeviceStream: %s", SDL_GetError());
        return;
    }
    SDL_ResumeAudioStreamDevice(g_stream);
}

void platform_audio_sdl_close(void) {
    if (g_stream) {
        SDL_DestroyAudioStream(g_stream);
        g_stream = NULL;
    }
}
