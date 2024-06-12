#include "chip8.h"

#include "SDL2/SDL.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>

#include <cstdint>
#include <iostream>

int main(int argc, char **argv) {

  if (argc != 2) {
    std::cerr << "Incorrect Number of arguments" << std::endl;
    return 1;
  }

  Chip8 chip8;

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

  std::uint8_t keymap[16] = {
      SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_q, SDLK_w, SDLK_e, SDLK_r,
      SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_z, SDLK_x, SDLK_c, SDLK_v,
  };

  if (!chip8.loadRom(argv[1])) {
    std::cerr << "Failed to load ROM: " << argv[1] << std::endl;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 2;
  }

  SDL_Event e;
  bool isAppRunning = true;
  while (isAppRunning) {
    chip8.emulateCycle();

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

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        isAppRunning = false;
      }

      if (e.type == SDL_KEYDOWN) {
        for (int i = 0; i < 16; ++i) {
          if (e.key.keysym.sym == keymap[i]) {
            chip8.keyboard[i] = 1;
          }
        }
      }

      if (e.type == SDL_KEYUP) {
        for (int i = 0; i < 16; ++i) {
          if (e.key.keysym.sym == keymap[i]) {
            chip8.keyboard[i] = 0;
          }
        }
      }
    }
    SDL_Delay(1000 / 60);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
