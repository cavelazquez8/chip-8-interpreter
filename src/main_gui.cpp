#include <iostream>
#include <cstdlib>

#include "gui_application.h"
#include "version.h"

int main(int argc, char* argv[]) {
    std::cout << "CHIP-8 Interpreter 1.2.0" << std::endl;
    std::cout << "Professional CHIP-8 Emulator with GUI" << std::endl;
    std::cout << "Build: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << std::endl;

    GuiApplication app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return EXIT_FAILURE;
    }
    
    app.run();
    app.shutdown();
    
    return EXIT_SUCCESS;
}