#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 180;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;


int run_game(SDL_Renderer *renderer);

void main_loop();

// flag for the game loop
bool running = false;
// SDL Events to make our program interactive
SDL_Event evt;


int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

#ifdef __EMSCRIPTEN__
    if (SDL_SetHintWithPriority(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas",
                                SDL_HINT_OVERRIDE) < 0) {
        printf("SDL_SetHint failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
#endif
    // Setup window
    SDL_Window *window =
            SDL_CreateWindow("SDL Demo!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Setup renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == nullptr) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Setup and run the game
    run_game(renderer);
    // Cleanup
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    // Done!
    return EXIT_SUCCESS;
}

int run_game(SDL_Renderer *renderer) {
    float deltaTime = 0.01;
    auto currentTime = (float) SDL_GetTicks();
    float accumulator = 0.0f;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create context with renderer and physics world
    struct Context {
        SDL_Renderer *renderer;
    };

// Setup gameplay loops
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#endif
#ifndef __EMSCRIPTEN__
    running = true;
    while (running) {
        auto newTime = (float) SDL_GetTicks();
        float frameTime = newTime - currentTime;

        // Prevent from going too fast
        if (frameTime > SCREEN_TICKS_PER_FRAME) { frameTime = SCREEN_TICKS_PER_FRAME; }

        currentTime = newTime;
        accumulator += frameTime;

        while (accumulator >= deltaTime) {
            main_loop();
            accumulator -= deltaTime;
        }

        // Render the rectangles to the screen
        SDL_RenderPresent(renderer);


        // Check If user want to quit
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
                case SDL_QUIT:
#ifdef __EMSCRIPTEN__
                    emscripten_cancel_main_loop();
#endif
#ifndef __EMSCRIPTEN__
                    running = false;
#endif
                    break;
            }
        }
    }
#endif

    return 0;
}

void main_loop() {
    // Check If user want to quit
    while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
            case SDL_QUIT:
#ifdef __EMSCRIPTEN__
                emscripten_cancel_main_loop();
#endif
#ifndef __EMSCRIPTEN__
                running = false;
#endif
                break;
        }
    }
}
