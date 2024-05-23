#include "SDL2/SDL.h"
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>

#include "chip8.h"

#include <SDL2/SDL_video.h>
#include <iostream>

int main(int arc, char **argv) {
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError();
    return 1;
  }

  window =
      SDL_CreateWindow("Chip8", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    std::cerr << "Window could not be created! SDL Error:" << SDL_GetError();
    SDL_Quit();
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    // Handle error
    std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_Event e;

  bool isAppRunning = true;

  while (isAppRunning) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        isAppRunning = false;
      }
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
