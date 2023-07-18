#include "flecs.h"
#include <SDL2/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define MOVE_SPEED 500

typedef struct Vector2 {
    float x;
    float y;
} Position, Size2;

int run_game(SDL_Renderer *renderer);
void main_loop();

// flag for the gameloop
bool running = false;
// Create a world
flecs::world world;
// SDL Events to make our program interactive
SDL_Event evt;

int main(int argc, char *argv[]) {
#ifdef __EMSCRIPTEN__
    SDL_SetHintWithPriority(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas", SDL_HINT_OVERRIDE);
#endif
    // Setup window
    SDL_Window *window = SDL_CreateWindow(
            "Flecs Web Demo!",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            800, 600,
            SDL_WINDOW_RESIZABLE);
    // Setup renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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
    // Set Sdl renderer as ecs context
    world.set_context(renderer);

    flecs::entity OnUpdate = world.entity()
                                    .add(flecs::Phase)
                                    .depends_on(flecs::OnUpdate);
    flecs::entity BeforeDraw = world.entity()
                                      .add(flecs::Phase)
                                      .depends_on(OnUpdate);
    flecs::entity OnDraw = world.entity()
                                   .add(flecs::Phase)
                                   .depends_on(BeforeDraw);
    flecs::entity AfterDraw = world.entity()
                                      .add(flecs::Phase)
                                      .depends_on(OnDraw);

    flecs::entity Player = world.entity();

    Player.set<Position>({10, 20})
            .set<Size2>({1, 2});

    world.system<Position>("MovePlayer").kind(OnUpdate).iter([](flecs::iter &it, Position *p) {
        // Get keyboard state
        const Uint8 *keystate = SDL_GetKeyboardState(nullptr);
        for (auto i: it) {
            if (keystate[SDL_SCANCODE_UP]) {
                p[i].y -= MOVE_SPEED * it.delta_time();
            }
            if (keystate[SDL_SCANCODE_DOWN]) {
                p[i].y += MOVE_SPEED * it.delta_time();
            }
            if (keystate[SDL_SCANCODE_LEFT]) {
                p[i].x -= MOVE_SPEED * it.delta_time();
            }
            if (keystate[SDL_SCANCODE_RIGHT]) {
                p[i].x += MOVE_SPEED * it.delta_time();
            }
        }
    });

    world.system<Position, Size2>("DrawPlayer").kind(OnDraw).iter([](flecs::iter &it, Position *p, Size2 *s) {
        SDL_Renderer *renderer = static_cast<SDL_Renderer *>(it.world().get_context());
        // Drawing each player
        for (auto i: it) {
            // Creating a new Rectangle
            SDL_Rect r;
            r.x = p[i].x;
            r.y = p[i].y;
            r.w = s[i].x;
            r.h = s[i].y;
            // Set the rectangle color to red
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            // Render the rectangle
            SDL_RenderFillRect(renderer, &r);
        }
    });

    world.system("SetupDraw").kind(BeforeDraw).iter([](flecs::iter &it) {
        SDL_Renderer *renderer = static_cast<SDL_Renderer *>(it.world().get_context());
        // Clearing the current render target with black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
        SDL_RenderClear(renderer);
    });

    world.system("PresentDraw").kind(AfterDraw).iter([](flecs::iter &it) {
        SDL_Renderer *renderer = static_cast<SDL_Renderer *>(it.world().get_context());
        // Render the rectangles to the screen
        SDL_RenderPresent(renderer);
    });

// Setup gameplay loops
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#endif
#ifndef __EMSCRIPTEN__
    running = true;
    while (running) { main_loop(); }
#endif
    // Cleanup flecs
    return ecs_fini(world);
}

void main_loop() {
    // Advance Game world
    world.progress(0);
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
