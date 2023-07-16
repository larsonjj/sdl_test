#include "Window.h"
#include "SDL.h"

#include <stdexcept>

Window::Window(const std::string &title, int width, int height, unsigned int flags) : m_window(nullptr,
                                                                                               SDL_DestroyWindow) {
    //Initialize SDL2 with appropriate subsystems.
    SDL_Init(SDL_INIT_VIDEO);

    auto window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
    if (window) {
        m_window.reset(window);
    } else {
        throw std::runtime_error(SDL_GetError());
    }
}

Window::~Window() {
    // Ensure that SDL_Quit is called even if the window is destroyed
    SDL_Quit();
}

std::optional<SDL_Event> Window::pollEvent() {
    SDL_Event event;
    if (SDL_PollEvent(&event))
        return event;
    else
        return {};
}
