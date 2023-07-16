#include "Window.h"
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void handleInput(bool &shouldQuit) {
    std::optional<SDL_Event> event;
    while ((event = Window::pollEvent()))
        switch (event->type) {
            case SDL_QUIT:
                shouldQuit = true;
                break;
        }
}

void updateFrame(bool &shouldQuit) {
    // Input
    handleInput(shouldQuit);
    // Update
    // Render
}

int main(int argc, char* args[]) {
    try {
        // Application Start
        Window window("SDL Test");

        // Application Loop
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(updateFrame, 0, 1);
#else
        bool shouldQuit = false;
        while (!shouldQuit) {
            updateFrame(shouldQuit)
        }
#endif
    } catch (const std::exception &exception) {
        auto errorMessage = exception.what();
        SDL_LogCritical(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "%s", errorMessage);
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "Error Message", errorMessage, nullptr);
        SDL_Quit();
        return 1;
    }

    // Must be called even if all subsystems are shut down
    SDL_Quit();
    return 0;
}
