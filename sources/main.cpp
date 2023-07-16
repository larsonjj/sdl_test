#include <iostream>
#include <fmt/core.h>
#include <SDL.h>

int main(int argc, char* args[]) {
    //Initialize SDL2 with appropriate subsystems.
    SDL_Init(SDL_INIT_VIDEO);
    fmt::print("Hello, world!\n");
    std::cout << "Hello, World!" << std::endl;
    SDL_Quit();
    return 0;
}
