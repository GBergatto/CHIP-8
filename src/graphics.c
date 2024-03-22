#include "graphics.h"
#include "chip8.h"

int init_sdl(sdl_t *sdl, const config_t config) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  sdl->window = SDL_CreateWindow(
      "SDL Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      config.window_width * config.scale_factor,
      config.window_height * config.scale_factor, SDL_WINDOW_SHOWN);
  if (sdl->window == NULL) {
    fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
    return 1;
  }

  sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
  if (sdl->renderer == NULL) {
    fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
    return 1;
  }

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

void update_screen(const sdl_t sdl, const config_t config,
                   const chip8_t chip8) {
  SDL_Rect rect = {
      .x = 0, .y = 0, .w = config.scale_factor, .h = config.scale_factor};

  // Get color values
  const uint8_t bg_r = (config.bg_color >> 24) & 0xFF;
  const uint8_t bg_g = (config.bg_color >> 16) & 0xFF;
  const uint8_t bg_b = (config.bg_color >> 8) & 0xFF;
  const uint8_t bg_a = (config.bg_color >> 0) & 0xFF;

  const uint8_t fg_r = (config.fg_color >> 24) & 0xFF;
  const uint8_t fg_g = (config.fg_color >> 16) & 0xFF;
  const uint8_t fg_b = (config.fg_color >> 8) & 0xFF;
  const uint8_t fg_a = (config.fg_color >> 0) & 0xFF;

  // Loop through display pixels
  for (uint32_t i = 0; i < sizeof chip8.display; i++) {
    rect.x = (i % config.window_width) * config.scale_factor;
    rect.y = (i / config.window_width) * config.scale_factor;

    if (chip8.display[i]) {
      SDL_SetRenderDrawColor(sdl.renderer, fg_r, fg_g, fg_b, fg_a);
      SDL_RenderFillRect(sdl.renderer, &rect);

      // Draw border around active pixel
      if (config.pixel_outline) {
        SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
        SDL_RenderDrawRect(sdl.renderer, &rect);
      }
    } else {
      SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
      SDL_RenderFillRect(sdl.renderer, &rect);
    }
  }
  SDL_RenderPresent(sdl.renderer);
}

int quit_sdl(const sdl_t sdl) {
  SDL_DestroyRenderer(sdl.renderer);
  SDL_DestroyWindow(sdl.window);
  SDL_Quit();

  return 0;
}

// Chip8 keypad     QWERTY
// 123C             1234
// 456D             QWER
// 789E             ASDF
// A0BF             ZXCV
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
      case SDLK_SPACE:
        if (chip8->state == RUNNING) {
          puts("=== PAUSED ===");
          chip8->state = PAUSED;
        } else {
          chip8->state = RUNNING;
        }
        break;

      case SDLK_1:
        chip8->keypad[0x1] = true;
        break;
      case SDLK_2:
        chip8->keypad[0x2] = true;
        break;
      case SDLK_3:
        chip8->keypad[0x3] = true;
        break;
      case SDLK_4:
        chip8->keypad[0xC] = true;
        break;

      case SDLK_q:
        chip8->keypad[0x4] = true;
        break;
      case SDLK_w:
        chip8->keypad[0x5] = true;
        break;
      case SDLK_e:
        chip8->keypad[0x6] = true;
        break;
      case SDLK_r:
        chip8->keypad[0xD] = true;
        break;

      case SDLK_a:
        chip8->keypad[0x7] = true;
        break;
      case SDLK_s:
        chip8->keypad[0x8] = true;
        break;
      case SDLK_d:
        chip8->keypad[0x9] = true;
        break;
      case SDLK_f:
        chip8->keypad[0xE] = true;
        break;

      case SDLK_z:
        chip8->keypad[0xA] = true;
        break;
      case SDLK_x:
        chip8->keypad[0x0] = true;
        break;
      case SDLK_c:
        chip8->keypad[0xB] = true;
        break;
      case SDLK_v:
        chip8->keypad[0xF] = true;
        break;

      default:
        break;
      }
      break;

    case SDL_KEYUP:
      switch (event.key.keysym.sym) {
      case SDLK_1:
        chip8->keypad[0x1] = false;
        break;
      case SDLK_2:
        chip8->keypad[0x2] = false;
        break;
      case SDLK_3:
        chip8->keypad[0x3] = false;
        break;
      case SDLK_4:
        chip8->keypad[0xC] = false;
        break;

      case SDLK_q:
        chip8->keypad[0x4] = false;
        break;
      case SDLK_w:
        chip8->keypad[0x5] = false;
        break;
      case SDLK_e:
        chip8->keypad[0x6] = false;
        break;
      case SDLK_r:
        chip8->keypad[0xD] = false;
        break;

      case SDLK_a:
        chip8->keypad[0x7] = false;
        break;
      case SDLK_s:
        chip8->keypad[0x8] = false;
        break;
      case SDLK_d:
        chip8->keypad[0x9] = false;
        break;
      case SDLK_f:
        chip8->keypad[0xE] = false;
        break;

      case SDLK_z:
        chip8->keypad[0xA] = false;
        break;
      case SDLK_x:
        chip8->keypad[0x0] = false;
        break;
      case SDLK_c:
        chip8->keypad[0xB] = false;
        break;
      case SDLK_v:
        chip8->keypad[0xF] = false;
        break;
      }
      break;

    default:
      break;
    }
  }
}
