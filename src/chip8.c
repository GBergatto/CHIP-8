#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

int init_sdl() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  if (init_sdl() != 0) {
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
