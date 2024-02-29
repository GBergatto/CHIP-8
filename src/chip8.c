#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
  uint8_t ram[4096];
  bool display[64 * 32]; // Default resolution
  uint16_t stack[12];    // Stack
  uint16_t I;            // Index register
  uint16_t PC;           // Program counter
  uint8_t V[16];         // Data registers
  uint8_t delay;         // Delay timer
  uint8_t sound;         // Sound timer
  bool keypad[16];       // Hexadecimal keypad
} chip8_t;

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

int init_chip8(chip8_t *chip8, const char *rom_name) {
  const uint32_t entrypoint = 0x200;
  const uint8_t font[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };

  // Load font
  memcpy(&chip8->ram[0x050], font, sizeof(font));

  // Open ROM file
  FILE *rom_file = fopen(rom_name, "rb");
  if (rom_file == NULL) {
    fprintf(stderr, "Could not open ROM file %s\n", rom_name);
    return 1;
  }

  // Get ROM size
  fseek(rom_file, 0, SEEK_END);
  const size_t rom_size = ftell(rom_file);
  const size_t max_size = sizeof chip8->ram - entrypoint;
  rewind(rom_file);

  if (rom_size > sizeof(chip8->ram)) {
    fprintf(stderr, "ROM file %s is too large! Size: %zu. Max size: %zu\n",
            rom_name, rom_size, max_size);
    return 1;
  }

  // Load ROM
  size_t ret =
      fread(&chip8->ram[entrypoint], sizeof(uint8_t), rom_size, rom_file);
  if (ret != rom_size) {
    fprintf(stderr, "Could not load ROM into RAM\n");
    return 1;
  }
  fclose(rom_file);

  chip8->state = RUNNING;
  chip8->PC = entrypoint;
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
      case SDLK_SPACE:
        if (chip8->state == RUNNING) {
          puts("=== PAUSED ===");
          chip8->state = PAUSED;
        } else {
          chip8->state = RUNNING;
        }
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

void emulate_instruction(chip8_t *chip8) {}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  sdl_t sdl;
  config_t config;
  chip8_t chip8;

  // Initialize emulator configuration
  config = (config_t){
      .window_width = 64,
      .window_height = 32,
      .scale_factor = 20,
      .fg_color = 0xFFFFFFFF,
      .bg_color = 0x000000FF,
  };

  // Initialize SDL
  if (init_sdl(&sdl, config) != 0) {
    exit(EXIT_FAILURE);
  }

  // Initialize Chip8
  if (init_chip8(&chip8, argv[1]) != 0) {
    exit(EXIT_FAILURE);
  }

  clear_screen(sdl, config);

  // Main emulator loop
  while (chip8.state != QUIT) {
    // Handle user input
    handle_input(&chip8);

    if (chip8.state == PAUSED)
      continue;

    // Emulate CHIP8 instructions
    emulate_instruction(&chip8);

    SDL_Delay(16);

    SDL_RenderPresent(sdl.renderer);
  }

  quit_sdl(sdl);

  exit(EXIT_SUCCESS);
}
