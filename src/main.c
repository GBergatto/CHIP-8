#include "chip8.h"
#include "graphics.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  sdl_t sdl = {0};
  config_t config = {0};
  chip8_t chip8 = {0};

  // Initialize emulator configuration
  config = (config_t){
      .window_width = 64,
      .window_height = 32,
      .scale_factor = 20,
      .fg_color = 0xFFFFFFFF,
      .bg_color = 0x000000FF,
      .pixel_outline = true,
      .shift_VX_only = false,
      .use_BXNN = false,
      .insts_per_sec = 500,
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

    const uint64_t start_time = SDL_GetPerformanceCounter();

    // Emulate CHIP8 instructions
    for (uint32_t i = 0; i < config.insts_per_sec / 60; i++) {
      emulate_instruction(&chip8, config);
    }

    const uint64_t end_time = SDL_GetPerformanceCounter();
    double time_elapsed = ((double)(end_time - start_time) * 1000) / SDL_GetPerformanceFrequency();

    // Run at approximately 60Hz
    SDL_Delay((16.67f > time_elapsed) ? 16.67f - time_elapsed : 0);

    update_screen(sdl, config, chip8);
    update_timers(&chip8);
  }

  quit_sdl(sdl);

  exit(EXIT_SUCCESS);
}
