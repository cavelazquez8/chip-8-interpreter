#include <SDL2/SDL_video.h>
#include <iostream>

#include "SDL2/SDL.h"

#include "chip8.h"

int main(int arc, char **argv) {

  int w = 1024;
  int h = 512;

  SDL_Window *window = NULL;

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    exit(1);
  }

  window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    exit(2);
  }

  Chip8 chip8;

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  // Set some test pixels in the frame buffer

  return 0;
}
