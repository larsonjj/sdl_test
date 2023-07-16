#include <iostream>
#include <SDL.h>

int main(int argc, char* args[]) {
    //Initialize SDL2 with appropriate subsystems.
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Log("Hello, world!\n");
    SDL_Quit();
    return 0;
}
