#include "flecs.h"
#include <SDL2/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const int MOVE_SPEED = 500;

struct Position {
    float x;
    float y;
};

struct Size2 {
    float x;
    float y;
};


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
    SDL_Window *window =
            SDL_CreateWindow("Flecs Web Demo!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
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

    flecs::entity OnUpdate = world.entity().add(flecs::Phase).depends_on(flecs::OnUpdate);
    flecs::entity BeforeDraw = world.entity().add(flecs::Phase).depends_on(OnUpdate);
    flecs::entity OnDraw = world.entity().add(flecs::Phase).depends_on(BeforeDraw);
    flecs::entity AfterDraw = world.entity().add(flecs::Phase).depends_on(OnDraw);

    flecs::entity Player = world.entity();

    Player.set<Position>({20, 20}).set<Size2>({32, 32});

    world.system<Position>("MovePlayer").kind(OnUpdate).iter([](flecs::iter &it, Position *p) {
        // Get keyboard state
        const Uint8 *keystate = SDL_GetKeyboardState(nullptr);
        // movement peed limiter
        float speed_limiter = 1.0;
        for (auto i: it) {
            if ((keystate[SDL_SCANCODE_UP] && keystate[SDL_SCANCODE_LEFT]) ||
                (keystate[SDL_SCANCODE_UP] && keystate[SDL_SCANCODE_RIGHT]) ||
                (keystate[SDL_SCANCODE_DOWN] && keystate[SDL_SCANCODE_RIGHT]) ||
                (keystate[SDL_SCANCODE_DOWN] && keystate[SDL_SCANCODE_LEFT])) {
                speed_limiter = 0.707;
            }

            if (keystate[SDL_SCANCODE_UP]) {
                p[i].y -= MOVE_SPEED * speed_limiter * it.delta_time();
            }
            if (keystate[SDL_SCANCODE_DOWN]) {
                p[i].y += MOVE_SPEED * speed_limiter * it.delta_time();
            }
            if (keystate[SDL_SCANCODE_LEFT]) {
                p[i].x -= MOVE_SPEED * speed_limiter * it.delta_time();
            }
            if (keystate[SDL_SCANCODE_RIGHT]) {
                p[i].x += MOVE_SPEED * speed_limiter * it.delta_time();
            }
        }
    });

    world.system<Position, Size2>("DrawPlayer")
            .kind(OnDraw)
            .iter([](flecs::iter &it, Position *p, Size2 *s) {
                SDL_Renderer *renderer = static_cast<SDL_Renderer *>(it.world().get_context());
                // Drawing each player
                for (auto i: it) {
                    // Creating a new Rectangle
                    SDL_Rect r;
                    r.x = (int) p[i].x;
                    r.y = (int) p[i].y;
                    r.w = (int) s[i].x;
                    r.h = (int) s[i].y;
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
    ecs_quit(world);

    return 0;
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
