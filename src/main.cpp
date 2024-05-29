#include "SDL2/SDL.h"
#include "chip8.h"
#include <iostream>

int main(int argc, char **argv) {
  SDL_Window *window{nullptr};
  SDL_Renderer *renderer{nullptr};

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError();
    return 1;
  }

  window =
      SDL_CreateWindow("Chip8", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
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

  SDL_Color colors[2] = {
      {0, 0, 0, 255},      // Black
      {255, 255, 255, 255} // White
  };
  SDL_Palette *palette = SDL_AllocPalette(2);
  SDL_SetPaletteColors(palette, colors, 0, 2);

  SDL_Surface *surface =
      SDL_CreateRGBSurfaceWithFormat(0, 64, 32, 8, SDL_PIXELFORMAT_INDEX8);
  SDL_SetSurfacePalette(surface, palette);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

  if (texture == nullptr) {
    std::cerr << "Texture could not be created! SDL Error: " << SDL_GetError();
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_Event e;
  bool isAppRunning = true;

  Chip8 chip8;
  chip8.setIndexRegister(0);    // Font data for '0' is at memory location 0
  chip8.setRegisterAt(0, 0);    // V0 = 0 (X coordinate)
  chip8.setRegisterAt(1, 0);    // V1 = 0 (Y coordinate)
  chip8.setMemory(0x200, 0xD0); // DXYN opcode (D000 in memory)
  chip8.setMemory(0x201, 0x05); // Draw 5-byte sprite (height of font)
  chip8.emulateCycle();

  while (isAppRunning) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        isAppRunning = false;
      }
    }

    if (chip8.getDrawFlag()) {
      uint8_t *pixels = static_cast<uint8_t *>(surface->pixels);
      for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
          pixels[y * 64 + x] = chip8.frameBuffer[y * 64 + x] ? 1 : 0;
        }
      }

      SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, nullptr, nullptr);
      SDL_RenderPresent(renderer);

      chip8.setDrawFlag(false); // Reset draw flag
    }

    SDL_Delay(2);             // Adjust delay as needed
    chip8.setDrawFlag(false); // Reset draw flag
  }

  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  SDL_FreePalette(palette);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
