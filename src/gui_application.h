#ifndef GUI_APPLICATION_H
#define GUI_APPLICATION_H

#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <vector>

#include "chip8.h"
#include "version.h"
#include "disassembler.h"

class GuiApplication {
public:
    GuiApplication();
    ~GuiApplication();

    GuiApplication(const GuiApplication&) = delete;
    GuiApplication& operator=(const GuiApplication&) = delete;
    GuiApplication(GuiApplication&&) = delete;
    GuiApplication& operator=(GuiApplication&&) = delete;

    bool initialize();
    void run();
    void shutdown();

private:
    // SDL and OpenGL
    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;
    bool running_ = true;

    // Emulator
    std::unique_ptr<Chip8> emulator_;
    bool emulator_running_ = false;
    bool emulator_paused_ = false;
    std::string current_rom_path_;
    
    // GUI State
    bool show_memory_viewer_ = true;
    bool show_registers_ = true;
    bool show_stack_viewer_ = true;
    bool show_disassembler_ = true;
    bool show_performance_ = true;
    bool show_about_dialog_ = false;
    bool show_settings_dialog_ = false;
    bool show_file_browser_ = false;
    
    // OpenGL resources
    unsigned int display_texture_ = 0;
    
    // Error dialog state
    bool show_error_dialog_ = false;
    std::string error_title_;
    std::string error_message_;
    
    // Performance tracking
    float fps_ = 0.0f;
    float frame_time_ = 0.0f;
    std::vector<float> fps_history_;
    
    // Settings
    float emulation_speed_ = 1.0f;
    bool vsync_enabled_ = true;
    int display_scale_ = 10;
    
    // Recent files
    std::vector<std::string> recent_files_;
    static constexpr size_t MAX_RECENT_FILES = 10;

    // Initialization
    bool initializeSDL();
    bool initializeImGui();
    void setupImGuiStyle();
    
    // Main loop
    void handleEvents();
    void update();
    void render();
    
    // GUI Panels
    void renderMenuBar();
    void renderToolbar();
    void renderEmulatorDisplay();
    void renderMemoryViewer();
    void renderRegistersPanel();
    void renderStackViewer();
    void renderDisassembler();
    void renderPerformancePanel();
    void renderStatusBar();
    
    // Dialogs
    void renderAboutDialog();
    void renderSettingsDialog();
    void renderFileLoadDialog();
    void renderErrorDialog();
    
    // Emulator control
    void loadROM(const std::string& path);
    void resetEmulator();
    void stepEmulator();
    void togglePause();
    
    // File management
    void addToRecentFiles(const std::string& path);
    void loadRecentFiles();
    void saveRecentFiles();
    
    // Utility
    void showErrorDialog(const std::string& title, const std::string& message);
    std::string formatBytes(size_t bytes);
    std::string formatTime(float seconds);
};

#endif // GUI_APPLICATION_H