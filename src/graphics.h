#ifndef MY_GRAPHICS
#define MY_GRAPHICS

#include <SDL2/SDL.h>

#include "chip8.h"

// SDL Container
typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_AudioSpec desired;
  SDL_AudioSpec obtained;
  SDL_AudioDeviceID dev;
} sdl_t;

int init_sdl(sdl_t *sdl, config_t *config);
void clear_screen(const sdl_t sdl, const config_t config);
void update_screen(const sdl_t sdl, const config_t config, const chip8_t chip8);
void update_timers(const sdl_t sdl, chip8_t *chip8);
void handle_input(chip8_t *chip8);
int quit_sdl(const sdl_t sdl);

#endif
