#include "gui_application.h"

#include <GL/gl.h>
#include <SDL2/SDL_opengl.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl2.h"

GuiApplication::GuiApplication() : emulator_(std::make_unique<Chip8>()) {
    fps_history_.reserve(100);
}

GuiApplication::~GuiApplication() {
    shutdown();
}

bool GuiApplication::initialize() {
    if (!initializeSDL()) {
        return false;
    }
    
    if (!initializeImGui()) {
        return false;
    }
    
    emulator_->init();
    loadRecentFiles();
    
    return true;
}

bool GuiApplication::initializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    // Use OpenGL 3.3 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    
    window_ = SDL_CreateWindow(
        "CHIP-8 Interpreter",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 800,
        window_flags);
        
    if (!window_) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }

    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_MakeCurrent(window_, gl_context_);
    SDL_GL_SetSwapInterval(vsync_enabled_ ? 1 : 0);

    return true;
}

bool GuiApplication::initializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    setupImGuiStyle();

    ImGui_ImplSDL2_InitForOpenGL(window_, gl_context_);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

void GuiApplication::setupImGuiStyle() {
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    
    // Professional color scheme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.30f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.35f, 0.40f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.30f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.35f, 0.40f, 1.00f);
}

void GuiApplication::run() {
    while (running_) {
        handleEvents();
        update();
        render();
    }
}

void GuiApplication::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        
        if (event.type == SDL_QUIT) {
            running_ = false;
        }
        
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(window_)) {
            running_ = false;
        }
        
        // Handle emulator input
        if (emulator_running_ && !emulator_paused_) {
            // Map SDL keys to CHIP-8 keys
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                bool pressed = (event.type == SDL_KEYDOWN);
                switch (event.key.keysym.sym) {
                    case SDLK_1: emulator_->setKeyState(0x1, pressed); break;
                    case SDLK_2: emulator_->setKeyState(0x2, pressed); break;
                    case SDLK_3: emulator_->setKeyState(0x3, pressed); break;
                    case SDLK_4: emulator_->setKeyState(0xC, pressed); break;
                    case SDLK_q: emulator_->setKeyState(0x4, pressed); break;
                    case SDLK_w: emulator_->setKeyState(0x5, pressed); break;
                    case SDLK_e: emulator_->setKeyState(0x6, pressed); break;
                    case SDLK_r: emulator_->setKeyState(0xD, pressed); break;
                    case SDLK_a: emulator_->setKeyState(0x7, pressed); break;
                    case SDLK_s: emulator_->setKeyState(0x8, pressed); break;
                    case SDLK_d: emulator_->setKeyState(0x9, pressed); break;
                    case SDLK_f: emulator_->setKeyState(0xE, pressed); break;
                    case SDLK_z: emulator_->setKeyState(0xA, pressed); break;
                    case SDLK_x: emulator_->setKeyState(0x0, pressed); break;
                    case SDLK_c: emulator_->setKeyState(0xB, pressed); break;
                    case SDLK_v: emulator_->setKeyState(0xF, pressed); break;
                }
            }
        }
    }
}

void GuiApplication::update() {
    static Uint64 last_time = SDL_GetPerformanceCounter();
    Uint64 current_time = SDL_GetPerformanceCounter();
    float delta_time = static_cast<float>(current_time - last_time) / SDL_GetPerformanceFrequency();
    last_time = current_time;
    
    // Update FPS
    fps_ = 1.0f / delta_time;
    frame_time_ = delta_time * 1000.0f;
    fps_history_.push_back(fps_);
    if (fps_history_.size() > 100) {
        fps_history_.erase(fps_history_.begin());
    }
    
    // Update emulator
    if (emulator_running_ && !emulator_paused_) {
        static float accumulator = 0.0f;
        accumulator += delta_time * emulation_speed_;
        
        // Run at approximately 540Hz (9 cycles per 60Hz frame)
        const float target_cycle_time = 1.0f / 540.0f;
        while (accumulator >= target_cycle_time) {
            emulator_->emulateCycle();
            accumulator -= target_cycle_time;
        }
    }
}

void GuiApplication::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    // Enable docking
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockSpaceOverViewport(0, viewport);
    
    renderMenuBar();
    renderToolbar();
    renderEmulatorDisplay();
    
    if (show_memory_viewer_) renderMemoryViewer();
    if (show_registers_) renderRegistersPanel();
    if (show_stack_viewer_) renderStackViewer();
    if (show_disassembler_) renderDisassembler();
    if (show_performance_) renderPerformancePanel();
    
    renderStatusBar();
    
    if (show_about_dialog_) renderAboutDialog();
    if (show_settings_dialog_) renderSettingsDialog();
    if (show_file_browser_) renderFileLoadDialog();
    if (show_error_dialog_) renderErrorDialog();
    
    // Rendering
    ImGui::Render();
    glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), 
               static_cast<int>(ImGui::GetIO().DisplaySize.y));
    glClearColor(0.10f, 0.10f, 0.10f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Handle multiple viewports
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
    
    SDL_GL_SwapWindow(window_);
}

void GuiApplication::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load ROM...", "Ctrl+O")) {
                show_file_browser_ = true;
            }
            
            if (ImGui::BeginMenu("Recent Files", !recent_files_.empty())) {
                for (const auto& file : recent_files_) {
                    if (ImGui::MenuItem(file.c_str())) {
                        loadROM(file);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Clear Recent")) {
                    recent_files_.clear();
                    saveRecentFiles();
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                running_ = false;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Emulation")) {
            if (ImGui::MenuItem("Reset", "Ctrl+R", false, emulator_running_)) {
                resetEmulator();
            }
            if (ImGui::MenuItem("Step", "F8", false, emulator_running_ && emulator_paused_)) {
                stepEmulator();
            }
            if (ImGui::MenuItem(emulator_paused_ ? "Resume" : "Pause", "F5", false, emulator_running_)) {
                togglePause();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Memory Viewer", nullptr, &show_memory_viewer_);
            ImGui::MenuItem("Registers", nullptr, &show_registers_);
            ImGui::MenuItem("Stack Viewer", nullptr, &show_stack_viewer_);
            ImGui::MenuItem("Disassembler", nullptr, &show_disassembler_);
            ImGui::MenuItem("Performance", nullptr, &show_performance_);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("Preferences...")) {
                show_settings_dialog_ = true;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                show_about_dialog_ = true;
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void GuiApplication::renderToolbar() {
    ImGui::Begin("Toolbar", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    
    if (ImGui::Button("Load ROM")) {
        show_file_browser_ = true;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Reset")) {
        resetEmulator();
    }
    ImGui::SameLine();
    
    if (ImGui::Button(emulator_paused_ ? "Resume" : "Pause")) {
        togglePause();
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Step")) {
        stepEmulator();
    }
    
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("Speed", &emulation_speed_, 0.1f, 5.0f, "%.1fx");
    
    ImGui::End();
}

void GuiApplication::renderEmulatorDisplay() {
    ImGui::Begin("Emulator Display");
    
    if (emulator_running_) {
        const auto& frame_buffer = emulator_->getFrameBuffer();
        
        // Create texture for display
        if (display_texture_ == 0) {
            glGenTextures(1, &display_texture_);
        }
        
        // Convert frame buffer to RGBA
        std::vector<uint32_t> pixels(Chip8::DISPLAY_SIZE);
        for (size_t i = 0; i < pixels.size(); ++i) {
            pixels[i] = frame_buffer[i] ? 0xFFFFFFFF : 0xFF000000;
        }
        
        glBindTexture(GL_TEXTURE_2D, display_texture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Chip8::DISPLAY_WIDTH, Chip8::DISPLAY_HEIGHT, 
                     0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        ImVec2 display_size(Chip8::DISPLAY_WIDTH * display_scale_, 
                           Chip8::DISPLAY_HEIGHT * display_scale_);
        ImGui::Image(reinterpret_cast<void*>(display_texture_), display_size);
    } else {
        ImGui::Text("No ROM loaded");
        if (ImGui::Button("Load ROM...")) {
            show_file_browser_ = true;
        }
    }
    
    ImGui::End();
}

void GuiApplication::renderMemoryViewer() {
    ImGui::Begin("Memory Viewer", &show_memory_viewer_);
    
    if (emulator_running_) {
        static int memory_base = 0x200;
        ImGui::InputInt("Base Address", &memory_base, 16, 16, ImGuiInputTextFlags_CharsHexadecimal);
        memory_base = std::clamp(memory_base, 0, static_cast<int>(Chip8::MEMORY_SIZE - 256));
        
        ImGui::BeginChild("MemoryHex", ImVec2(0, 0), true);
        
        for (int row = 0; row < 16; ++row) {
            ImGui::Text("%04X:", memory_base + row * 16);
            ImGui::SameLine();
            
            for (int col = 0; col < 16; ++col) {
                int address = memory_base + row * 16 + col;
                if (address < Chip8::MEMORY_SIZE) {
                    auto memory_value = emulator_->getMemoryAt(address);
                    if (memory_value) {
                        ImGui::SameLine();
                        ImGui::Text("%02X", *memory_value);
                    }
                }
            }
        }
        
        ImGui::EndChild();
    } else {
        ImGui::Text("No ROM loaded");
    }
    
    ImGui::End();
}

void GuiApplication::renderRegistersPanel() {
    ImGui::Begin("Registers", &show_registers_);
    
    if (emulator_running_) {
        ImGui::Columns(2, "RegisterColumns");
        
        for (int i = 0; i < 16; ++i) {
            auto reg_value = emulator_->getRegisterAt(i);
            if (reg_value) {
                ImGui::Text("V%X: 0x%02X (%d)", i, *reg_value, *reg_value);
                if (i == 7) ImGui::NextColumn();
            }
        }
        
        ImGui::Columns(1);
        ImGui::Separator();
        
        ImGui::Text("PC: 0x%04X", emulator_->getProgramCounter());
        ImGui::Text("I:  0x%04X", emulator_->getIndexRegister());
        ImGui::Text("SP: 0x%02X", emulator_->getStackPointer());
        ImGui::Text("DT: 0x%02X", emulator_->getDelayTimer());
        ImGui::Text("ST: 0x%02X", emulator_->getSoundTimer());
    } else {
        ImGui::Text("No ROM loaded");
    }
    
    ImGui::End();
}

void GuiApplication::renderStackViewer() {
    ImGui::Begin("Stack Viewer", &show_stack_viewer_);
    
    if (emulator_running_) {
        ImGui::Text("Stack Pointer: %d", emulator_->getStackPointer());
        ImGui::Separator();
        
        for (int i = 0; i < 16; ++i) {
            auto stack_value = emulator_->getStackAt(i);
            if (stack_value) {
                bool is_current = (i == emulator_->getStackPointer());
                if (is_current) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                }
                ImGui::Text("[%02d] 0x%04X", i, *stack_value);
                if (is_current) {
                    ImGui::PopStyleColor();
                }
            }
        }
    } else {
        ImGui::Text("No ROM loaded");
    }
    
    ImGui::End();
}

void GuiApplication::renderDisassembler() {
    ImGui::Begin("Disassembler", &show_disassembler_);
    
    if (emulator_running_) {
        std::uint16_t current_pc = emulator_->getProgramCounter();
        std::uint16_t start_address = (current_pc >= 20) ? current_pc - 20 : 0x200;
        
        // Disassemble around current PC
        auto memory_span = emulator_->getMemorySpan();
        auto instructions = Disassembler::disassembleMemory(
            memory_span.data(), start_address, 32, current_pc);
        
        ImGui::BeginChild("DisassemblyList", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        
        for (const auto& instr : instructions) {
            if (instr.is_current_pc) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                ImGui::Text("→");
                ImGui::PopStyleColor();
            } else {
                ImGui::Text(" ");
            }
            
            ImGui::SameLine();
            ImGui::Text("0x%04X: %04X  %-12s  %s", 
                       instr.address, instr.opcode, 
                       instr.mnemonic.c_str(), instr.description.c_str());
        }
        
        ImGui::EndChild();
    } else {
        ImGui::Text("No ROM loaded");
    }
    
    ImGui::End();
}

void GuiApplication::renderPerformancePanel() {
    ImGui::Begin("Performance", &show_performance_);
    
    ImGui::Text("FPS: %.1f (%.2f ms)", fps_, frame_time_);
    
    if (!fps_history_.empty()) {
        float min_fps = *std::min_element(fps_history_.begin(), fps_history_.end());
        float max_fps = *std::max_element(fps_history_.begin(), fps_history_.end());
        
        ImGui::PlotLines("FPS History", fps_history_.data(), fps_history_.size(),
                        0, nullptr, min_fps * 0.9f, max_fps * 1.1f, ImVec2(0, 80));
    }
    
    if (emulator_running_) {
        ImGui::Separator();
        ImGui::Text("Emulation Speed: %.1fx", emulation_speed_);
        ImGui::Text("State: %s", emulator_paused_ ? "Paused" : "Running");
    }
    
    ImGui::End();
}

void GuiApplication::renderStatusBar() {
    // Create a status bar window at the bottom
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | 
                                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    
    ImGui::SetNextWindowPos(ImVec2(work_pos.x, work_pos.y + work_size.y - 30));
    ImGui::SetNextWindowSize(ImVec2(work_size.x, 30));
    
    if (ImGui::Begin("StatusBar", nullptr, window_flags)) {
        ImGui::Text("FPS: %.0f", fps_);
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        
        if (emulator_running_) {
            ImGui::Text("ROM: %s", current_rom_path_.empty() ? "Unknown" : current_rom_path_.c_str());
            ImGui::SameLine();
            ImGui::Text("|");
            ImGui::SameLine();
            ImGui::Text("PC: 0x%04X", emulator_->getProgramCounter());
        } else {
            ImGui::Text("No ROM loaded");
        }
    }
    ImGui::End();
}

void GuiApplication::renderAboutDialog() {
    ImGui::OpenPopup("About");
    
    if (ImGui::BeginPopupModal("About", &show_about_dialog_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("CHIP-8 Interpreter");
        ImGui::Text("Version: 1.2.0");
        ImGui::Text("Professional CHIP-8 Emulator with GUI");
        ImGui::Separator();
        ImGui::Text("Built: %s %s", __DATE__, __TIME__);
        ImGui::Text("Copyright (c) 2024");
        
        if (ImGui::Button("Close")) {
            show_about_dialog_ = false;
        }
        
        ImGui::EndPopup();
    }
}

void GuiApplication::renderSettingsDialog() {
    if (ImGui::Begin("Settings", &show_settings_dialog_)) {
        if (ImGui::CollapsingHeader("Display")) {
            ImGui::SliderInt("Display Scale", &display_scale_, 1, 20);
            ImGui::Checkbox("VSync", &vsync_enabled_);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                SDL_GL_SetSwapInterval(vsync_enabled_ ? 1 : 0);
            }
        }
        
        if (ImGui::CollapsingHeader("Emulation")) {
            ImGui::SliderFloat("Speed", &emulation_speed_, 0.1f, 10.0f);
        }
        
        if (ImGui::Button("Close")) {
            show_settings_dialog_ = false;
        }
    }
    ImGui::End();
}

void GuiApplication::renderFileLoadDialog() {
    // Simple file browser - in a real implementation you'd use ImGuiFileDialog or similar
    if (ImGui::Begin("Load ROM", &show_file_browser_)) {
        ImGui::Text("ROM Files:");
        
        // List ROM files in the roms directory
        std::vector<std::string> rom_files = {"maze.ch8", "airplane.ch8", "connect4.ch8"};
        
        for (const auto& rom : rom_files) {
            if (ImGui::Selectable(rom.c_str())) {
                loadROM("../roms/" + rom);
                show_file_browser_ = false;
            }
        }
        
        if (ImGui::Button("Cancel")) {
            show_file_browser_ = false;
        }
    }
    ImGui::End();
}

void GuiApplication::renderErrorDialog() {
    ImGui::OpenPopup("Error");
    
    if (ImGui::BeginPopupModal("Error", &show_error_dialog_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", error_title_.c_str());
        ImGui::Separator();
        ImGui::TextWrapped("%s", error_message_.c_str());
        
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            show_error_dialog_ = false;
        }
        
        ImGui::EndPopup();
    }
}

void GuiApplication::loadROM(const std::string& path) {
    if (emulator_->loadRom(path)) {
        current_rom_path_ = path;
        emulator_running_ = true;
        emulator_paused_ = false;
        addToRecentFiles(path);
    } else {
        showErrorDialog("Load Error", "Failed to load ROM: " + path);
    }
}

void GuiApplication::resetEmulator() {
    if (emulator_running_) {
        emulator_->init();
        if (!current_rom_path_.empty()) {
            emulator_->loadRom(current_rom_path_);
        }
    }
}

void GuiApplication::stepEmulator() {
    if (emulator_running_ && emulator_paused_) {
        emulator_->emulateCycle();
    }
}

void GuiApplication::togglePause() {
    if (emulator_running_) {
        emulator_paused_ = !emulator_paused_;
    }
}

void GuiApplication::addToRecentFiles(const std::string& path) {
    auto it = std::find(recent_files_.begin(), recent_files_.end(), path);
    if (it != recent_files_.end()) {
        recent_files_.erase(it);
    }
    
    recent_files_.insert(recent_files_.begin(), path);
    
    if (recent_files_.size() > MAX_RECENT_FILES) {
        recent_files_.resize(MAX_RECENT_FILES);
    }
    
    saveRecentFiles();
}

void GuiApplication::loadRecentFiles() {
    // Simple implementation - in practice you'd load from a config file
    recent_files_.clear();
}

void GuiApplication::saveRecentFiles() {
    // Simple implementation - in practice you'd save to a config file
}

void GuiApplication::showErrorDialog(const std::string& title, const std::string& message) {
    error_title_ = title;
    error_message_ = message;
    show_error_dialog_ = true;
    
    // Also log to console for debugging
    std::cerr << title << ": " << message << std::endl;
}

std::string GuiApplication::formatBytes(size_t bytes) {
    if (bytes < 1024) return std::to_string(bytes) + " B";
    if (bytes < 1024 * 1024) return std::to_string(bytes / 1024) + " KB";
    return std::to_string(bytes / (1024 * 1024)) + " MB";
}

std::string GuiApplication::formatTime(float seconds) {
    if (seconds < 60) return std::to_string(static_cast<int>(seconds)) + "s";
    int minutes = static_cast<int>(seconds / 60);
    int secs = static_cast<int>(seconds) % 60;
    return std::to_string(minutes) + "m " + std::to_string(secs) + "s";
}

void GuiApplication::shutdown() {
    saveRecentFiles();
    
    // Clean up OpenGL resources
    if (display_texture_ != 0) {
        glDeleteTextures(1, &display_texture_);
        display_texture_ = 0;
    }
    
    if (gl_context_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        
        SDL_GL_DeleteContext(gl_context_);
        gl_context_ = nullptr;
    }
    
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    
    SDL_Quit();
}