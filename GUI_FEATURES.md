# CHIP-8 Interpreter GUI Features

## Overview

The CHIP-8 Interpreter now features a professional, modern GUI built with Dear ImGui. This provides a comprehensive debugging and development environment for CHIP-8 programs.

## Core Features

### Main Window Layout
- **Dockable Interface**: Professional layout with moveable, resizable panels
- **Menu Bar**: Complete file, emulation, view, settings, and help menus
- **Toolbar**: Quick access to common operations (Load ROM, Reset, Play/Pause, Step, Speed)
- **Status Bar**: Real-time display of FPS, ROM info, and program counter

### Emulator Display
- **Scaled Display**: Configurable scaling (1x to 20x) for the 64x32 CHIP-8 screen
- **Real-time Rendering**: Hardware-accelerated OpenGL rendering
- **Pixel-perfect Display**: Accurate representation of the original CHIP-8 graphics

### Debugging Panels

#### Memory Viewer
- **Hex Editor**: Browse and view the complete 4KB memory space
- **Address Navigation**: Jump to specific memory addresses
- **Real-time Updates**: Memory content updates as the emulator runs
- **Formatted Display**: Clear hex grid with address labels

#### Register Panel
- **All Registers**: Display of all 16 general-purpose registers (V0-VF)
- **Special Registers**: Program counter, index register, stack pointer
- **Timers**: Delay timer and sound timer values
- **Real-time Updates**: Values update as the emulator executes

#### Stack Viewer
- **Complete Stack**: Display of all 16 stack levels
- **Current Position**: Highlighted current stack pointer position
- **Call History**: Track subroutine calls and returns

#### Disassembler
- **Real-time Disassembly**: Live disassembly around current program counter
- **CHIP-8 Mnemonics**: Standard assembly mnemonics for all instructions
- **Current Instruction**: Highlighted current PC location
- **Instruction Descriptions**: Human-readable explanations for each opcode

#### Performance Monitor
- **FPS Display**: Real-time frames per second
- **Frame Time**: Millisecond timing information
- **FPS History**: Graphical plot of performance over time
- **Emulation Speed**: Current emulation speed multiplier

### ROM Management
- **File Browser**: Built-in ROM file selection
- **Recent Files**: Quick access to recently loaded ROMs
- **Drag & Drop**: (Planned) Direct file dropping support
- **ROM Information**: Display of loaded ROM details

### Emulation Controls
- **Play/Pause**: Toggle emulation execution
- **Reset**: Restart the current ROM
- **Step**: Single instruction stepping for debugging
- **Speed Control**: Adjustable emulation speed (0.1x to 5.0x)

### Settings & Configuration
- **Display Settings**: 
  - Configurable display scaling
  - VSync control
- **Emulation Settings**:
  - Speed adjustment
  - Timer frequencies
- **UI Settings**:
  - Panel visibility toggles
  - Theme options

### Professional Features
- **Multiple Viewports**: Support for multi-monitor setups
- **Persistent Layout**: Window positions and sizes are remembered
- **Keyboard Shortcuts**: Full keyboard navigation support
- **Error Handling**: Comprehensive error reporting with user-friendly messages
- **Professional Styling**: Dark theme with consistent modern appearance

## Keyboard Shortcuts

### Emulation Controls
- `Ctrl+O`: Load ROM
- `Ctrl+R`: Reset emulator
- `F5`: Play/Pause toggle
- `F8`: Step single instruction

### CHIP-8 Keypad
The original CHIP-8 16-key keypad is mapped to:
```
1 2 3 4    →    1 2 3 C
Q W E R    →    4 5 6 D
A S D F    →    7 8 9 E
Z X C V    →    A 0 B F
```

### View Controls
- `Alt+F4`: Exit application
- Panel toggles available through View menu

## Technical Implementation

### Architecture
- **Modern C++20**: Uses latest C++ features including std::span and std::optional
- **ImGui Docking**: Professional docking interface with Dear ImGui
- **OpenGL 3.3**: Hardware-accelerated rendering
- **SDL2**: Cross-platform window and input management
- **CMake**: Modern build system with proper dependency management

### Performance
- **60 FPS GUI**: Smooth user interface at 60 frames per second
- **540 Hz Emulation**: Accurate CHIP-8 timing (9 cycles per 60Hz frame)
- **Efficient Rendering**: Only updates when needed to minimize CPU usage
- **Memory Safety**: Modern C++ with bounds checking and RAII patterns

### Cross-Platform Support
- **Linux**: Native support with X11/Wayland
- **Windows**: Full Windows 10/11 support
- **macOS**: macOS 10.14+ support
- **High DPI**: Proper scaling on high-resolution displays

## Development Features

### Debugging Capabilities
- **Breakpoints**: (Planned) Set execution breakpoints
- **Memory Watch**: Real-time memory monitoring
- **Register History**: Track register changes over time
- **Call Stack**: Complete subroutine call tracking
- **Instruction Tracing**: Step-by-step execution analysis

### ROM Development Support
- **Quick Reload**: Fast ROM reloading for development iteration
- **Error Reporting**: Clear error messages for invalid operations
- **State Inspection**: Complete visibility into emulator state
- **Performance Profiling**: Built-in performance monitoring tools

## Future Enhancements

### Planned Features
- **Save States**: Save and load emulator states
- **Improved File Browser**: Native file dialogs
- **Help System**: Integrated documentation and tutorials
- **ROM Metadata**: Display ROM information and compatibility
- **Audio Support**: Visual representation of sound timer
- **Sprite Editor**: Built-in sprite editing tools
- **Custom Themes**: Additional UI themes and customization

### Development Roadmap
1. **Phase 1**: Core debugging features (Complete)
2. **Phase 2**: Enhanced file management and ROM support
3. **Phase 3**: Advanced debugging tools (breakpoints, watchpoints)
4. **Phase 4**: Development tools (sprite editor, ROM builder)
5. **Phase 5**: Community features (ROM sharing, tutorials)

## Getting Started

1. **Build the Application**:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

2. **Run the GUI**:
   ```bash
   ./src/chip8
   ```

3. **Load a ROM**:
   - Use `File → Load ROM...` or `Ctrl+O`
   - Select from recent files
   - Try the included sample ROMs in the `roms/` directory

4. **Start Debugging**:
   - Load a ROM
   - Use the debugging panels to inspect emulator state
   - Use `F8` to step through instructions
   - Monitor memory and registers in real-time

The GUI provides a complete development and debugging environment for CHIP-8 programming, making it easier than ever to understand, debug, and develop CHIP-8 applications.