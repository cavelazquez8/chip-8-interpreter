#include "SDL2/SDL.h"

#include "chip8.h"

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <iostream>

int main(int argc, char **argv) {
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError();
    return 1;
  }

  window =
      SDL_CreateWindow("Chip8", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 1024, 512, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    std::cerr << "Window could not be created! SDL Error: " << SDL_GetError();
    SDL_Quit();
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING, 64, 32);
  if (texture == nullptr) {
    std::cerr << "Texture could not be created! SDL Error: " << SDL_GetError();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  uint32_t pixels[2048];

  Chip8 chip8;

  bool isAppRunning = true;
  SDL_Event e;
  while (isAppRunning) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        isAppRunning = false;
      }
    }

    if (chip8.getDrawFlag()) {
      for (int i = 0; i < 2048; ++i) {
        uint8_t pixel = chip8.frameBuffer[i];
        pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
      }
      SDL_UpdateTexture(texture, nullptr, pixels, 64 * sizeof(Uint32));
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, nullptr, nullptr);
      SDL_RenderPresent(renderer);

      chip8.setDrawFlag(false);
    }

    chip8.setDrawFlag(false); // Reset draw flag
  }
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
