#include "chip8.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {
    constexpr int WINDOW_WIDTH = 1024;
    constexpr int WINDOW_HEIGHT = 512;
    constexpr int TARGET_FPS = 60;
    constexpr int FRAME_DELAY = 1000 / TARGET_FPS;
    constexpr int DISPLAY_WIDTH = 64;
    constexpr int DISPLAY_HEIGHT = 32;
    constexpr int DISPLAY_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT;
    
    constexpr std::array<SDL_Keycode, 16> KEYMAP = {
        SDLK_1, SDLK_2, SDLK_3, SDLK_4,
        SDLK_q, SDLK_w, SDLK_e, SDLK_r,
        SDLK_a, SDLK_s, SDLK_d, SDLK_f,
        SDLK_z, SDLK_x, SDLK_c, SDLK_v
    };

    struct SDLCleanup {
        ~SDLCleanup() { SDL_Quit(); }
    };

    class SDLRenderer {
    public:
        SDLRenderer() = default;
        ~SDLRenderer() {
            if (texture_) SDL_DestroyTexture(texture_);
            if (renderer_) SDL_DestroyRenderer(renderer_);
            if (window_) SDL_DestroyWindow(window_);
        }
        
        SDLRenderer(const SDLRenderer&) = delete;
        SDLRenderer& operator=(const SDLRenderer&) = delete;
        SDLRenderer(SDLRenderer&&) = delete;
        SDLRenderer& operator=(SDLRenderer&&) = delete;
        
        [[nodiscard]] bool initialize() {
            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
                return false;
            }
            
            window_ = SDL_CreateWindow("CHIP-8 Emulator", 
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
            if (!window_) {
                std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
                return false;
            }
            
            renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
            if (!renderer_) {
                std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
                return false;
            }
            
            texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_STREAMING, 
                                       DISPLAY_WIDTH, DISPLAY_HEIGHT);
            if (!texture_) {
                std::cerr << "Texture could not be created! SDL Error: " << SDL_GetError() << std::endl;
                return false;
            }
            
            return true;
        }
        
        void render(const Chip8& emulator) {
            if (!emulator.getDrawFlag()) return;
            
            std::array<std::uint32_t, DISPLAY_SIZE> pixels;
            const auto& frameBuffer = emulator.getFrameBuffer();
            
            for (std::size_t i = 0; i < pixels.size(); ++i) {
                pixels[i] = frameBuffer[i] ? 0xFFFFFFFF : 0xFF000000;
            }
            
            SDL_UpdateTexture(texture_, nullptr, pixels.data(), 
                            DISPLAY_WIDTH * sizeof(std::uint32_t));
            SDL_RenderClear(renderer_);
            SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
            SDL_RenderPresent(renderer_);
        }
        
    private:
        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;
        SDL_Texture* texture_ = nullptr;
    };

    void handleKeyEvent(const SDL_Event& event, Chip8& emulator) {
        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) return;
        
        const bool isPressed = (event.type == SDL_KEYDOWN);
        
        for (std::size_t i = 0; i < KEYMAP.size(); ++i) {
            if (event.key.keysym.sym == KEYMAP[i]) {
                emulator.setKeyState(static_cast<std::uint8_t>(i), isPressed);
                break;
            }
        }
    }

    void printUsage(std::string_view programName) {
        std::cerr << "Usage: " << programName << " <rom_file>" << std::endl;
        std::cerr << "Example: " << programName << " roms/maze.ch8" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    const SDLCleanup sdlCleanup;
    
    Chip8 emulator;
    
    if (!emulator.loadRom(argv[1])) {
        std::cerr << "Failed to load ROM: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }
    
    SDLRenderer renderer;
    if (!renderer.initialize()) {
        return EXIT_FAILURE;
    }

    SDL_Event event;
    bool running = true;
    
    while (running) {
        emulator.emulateCycle();
        
        // Check for emulator errors
        if (emulator.getLastError() != Chip8::ErrorCode::None) {
            std::cerr << "Emulator error: " << emulator.getLastErrorMessage() << std::endl;
            // Continue running but log the error
        }
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else {
                handleKeyEvent(event, emulator);
            }
        }
        
        renderer.render(emulator);
        if (emulator.getDrawFlag()) {
            emulator.setDrawFlag(false);
        }
        
        SDL_Delay(FRAME_DELAY);
    }
    
    return EXIT_SUCCESS;
}