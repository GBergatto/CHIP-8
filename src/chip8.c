#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

// SDL Container
typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
} sdl_t;

// Emulator configuration
typedef struct {
  uint32_t window_width;
  uint32_t window_height;
  uint32_t scale_factor;
  uint32_t fg_color;
  uint32_t bg_color;
} config_t;

// Emulator states
typedef enum {
  QUIT,
  RUNNING,
  PAUSED,
} emulator_state_t;

// CHIP8 machine
typedef struct {
  emulator_state_t state;
} chip8_t;

int init_sdl(sdl_t *sdl, const config_t config) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  sdl->window = SDL_CreateWindow(
      "SDL Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      config.window_width * config.scale_factor,
      config.window_height * config.scale_factor, SDL_WINDOW_SHOWN);
  if (sdl->window == NULL) {
    printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    return 1;
  }

  sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
  if (sdl->renderer == NULL) {
    printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
    return 1;
  }

  return 0;
}

int init_chip8(chip8_t *chip8) {
  chip8->state = RUNNING;
  return 0;
}

void clear_screen(const sdl_t sdl, const config_t config) {
  const uint8_t r = (config.bg_color >> 24) & 0xFF;
  const uint8_t g = (config.bg_color >> 16) & 0xFF;
  const uint8_t b = (config.bg_color >> 8) & 0xFF;
  const uint8_t a = (config.bg_color >> 0) & 0xFF;

  SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
  SDL_RenderClear(sdl.renderer);
}

int quit_sdl(const sdl_t sdl) {
  SDL_DestroyRenderer(sdl.renderer);
  SDL_DestroyWindow(sdl.window);
  SDL_Quit();

  return 0;
}

void handle_input(chip8_t *chip8) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT: // Quit emulator
      chip8->state = QUIT;
      return;

    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE: // Quit emulator
        chip8->state = QUIT;
        break;
      default:
        break;
      }
      break;

    case SDL_KEYUP:
      break;

    default:
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  sdl_t sdl;
  config_t config;
  chip8_t chip8;

  // Initialize emulator configuration
  config = (config_t){
      .window_width = 64,
      .window_height = 32,
      .scale_factor = 20,
      .fg_color = 0xFFFFFFFF,
      .bg_color = 0xFFFF00FF,
  };

  // Initialize SDL
  if (init_sdl(&sdl, config) != 0) {
    exit(EXIT_FAILURE);
  }

  // Initialize Chip8
  if (init_chip8(&chip8) != 0) {
    exit(EXIT_FAILURE);
  }

  clear_screen(sdl, config);

  // Main emulator loop
  while (chip8.state != QUIT) {
    // Handle user input
    handle_input(&chip8);

    // Emulate CHIP8 instructions

    SDL_Delay(16);

    SDL_RenderPresent(sdl.renderer);
  }

  quit_sdl(sdl);

  exit(EXIT_SUCCESS);
}
