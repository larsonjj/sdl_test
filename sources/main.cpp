#include "flecs.h"
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 180;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const int MOVE_SPEED = 100;

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

// flag for the game loop
bool running = false;
// Create a world
flecs::world world;
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
            SDL_CreateWindow("Flecs Web Demo!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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

    // Set Sdl renderer as ecs context
    world.set_context(new Context({renderer}));

    flecs::entity OnUpdate = world.entity().add(flecs::Phase).depends_on(flecs::OnUpdate);
    flecs::entity BeforeDraw = world.entity().add(flecs::Phase).depends_on(OnUpdate);
    flecs::entity OnDraw = world.entity().add(flecs::Phase).depends_on(BeforeDraw);

    flecs::entity Player = world.entity();

    Player.set<Position>({20, 20}).set<Size2>({8, 8});

    world.system<Position>("MovePlayer").kind(OnUpdate).iter([](flecs::iter &it, Position *p) {
        // Get keyboard state
        const Uint8 *key_state = SDL_GetKeyboardState(nullptr);
        // movement speed limiter
        float speed_limiter = 1.0;
        for (auto i: it) {
            if ((key_state[SDL_SCANCODE_UP] && key_state[SDL_SCANCODE_LEFT]) ||
                (key_state[SDL_SCANCODE_UP] && key_state[SDL_SCANCODE_RIGHT]) ||
                (key_state[SDL_SCANCODE_DOWN] && key_state[SDL_SCANCODE_RIGHT]) ||
                (key_state[SDL_SCANCODE_DOWN] && key_state[SDL_SCANCODE_LEFT])) {
                speed_limiter = 0.707;
            }

            if (key_state[SDL_SCANCODE_UP]) {
                p[i].y -= MOVE_SPEED * speed_limiter * it.delta_time();
            }
            if (key_state[SDL_SCANCODE_DOWN]) {
                p[i].y += MOVE_SPEED * speed_limiter * it.delta_time();
            }
            if (key_state[SDL_SCANCODE_LEFT]) {
                p[i].x -= MOVE_SPEED * speed_limiter * it.delta_time();
            }
            if (key_state[SDL_SCANCODE_RIGHT]) {
                p[i].x += MOVE_SPEED * speed_limiter * it.delta_time();
            }
        }
    });

    world.system<Position, Size2>("DrawPlayer")
            .kind(OnDraw)
            .iter([](flecs::iter &it, Position *p, Size2 *s) {
                auto context = static_cast<Context *>(it.world().get_context());
                SDL_Renderer *renderer = context->renderer;

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
        auto context = static_cast<Context *>(it.world().get_context());
        SDL_Renderer *renderer = context->renderer;
        // Clearing the current render target with black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
        SDL_RenderClear(renderer);
    });

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
    // Cleanup flecs
    ecs_quit(world);

    return 0;
}

void main_loop() {
    // Advance Game world
    world.progress();
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
