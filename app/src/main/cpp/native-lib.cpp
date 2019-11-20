#include <jni.h>
#include <string>
#include <android/log.h>
#include <SDL_ttf.h>
#include <unistd.h>
#include "hqm.h"

#define DELAY (150)

int request_user_groups(TTF_Font *font, SDL_Renderer *renderer);

int
renderText(TTF_Font *font, int x, int y, const char *text, SDL_Renderer *renderer);

void request_user_groups(SDL_Renderer *pRenderer);

void start_hqm(TTF_Font *font, SDL_Renderer *renderer);

int retryerYPos = 300;

int SDL_main(int argc, char *argv[]) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    window = SDL_CreateWindow(
            "An SDL2 HQM example",             // window title
            SDL_WINDOWPOS_UNDEFINED,           // initial x position
            SDL_WINDOWPOS_UNDEFINED,           // initial y position
            1080,                               // width, in pixels
            1920,                               // height, in pixels
            SDL_WINDOW_OPENGL                  // flags - see below
    );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    // Set render color to red ( background will be rendered in this color )
    SDL_SetRenderDrawColor(renderer, 70, 15, 15, 255);

    // Clear window
    SDL_RenderClear( renderer );

    // Initialize TTF
    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init: %s\n", TTF_GetError());
        return 7;
    }

    // Load font
    TTF_Font *font = TTF_OpenFont("FreeSans.ttf", 32);
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Unable to load font: %s\n", TTF_GetError());
        return 8;
    }

    /// HQM
    start_hqm(font, renderer);
    ///

    bool quit = false;
    SDL_Event event;
    while(!quit && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                if ((event.key.keysym.sym == SDLK_AC_BACK) || (event.key.keysym.sym == SDLK_ESCAPE)) {
                    quit = true;
                }
                break;

            case SDL_FINGERDOWN: {
                renderText(font, 0, retryerYPos += 50, "Retrying...", renderer);
                request_user_groups(font, renderer);
                break;
            }
            default:
                break;
        }
    }

    return 0;
}

void start_hqm(TTF_Font *font, SDL_Renderer *renderer) {
    renderText(font, 0, 0, "Initializing HQM SDK: ...", renderer);
    // init
    hqm_init(
            "38e44d7", // sdk key
            1);         // enable debug mode

    renderText(font, 0, 0, "Initializing HQM SDK: OK", renderer);

    SDL_Delay(DELAY);

    renderText(font, 0, 50, "Starting apps collector: ...", renderer);
    // start sdk
    hqm_start();
    renderText(font, 0, 50, "Starting apps collector: OK", renderer);

    SDL_Delay(DELAY);

    renderText(font, 0, 100, "Logging event 1: ...", renderer);
    // log event 1
    hqm_log("test_event", "just a string");
    renderText(font, 0, 100, "Logging event 1: OK", renderer);

    SDL_Delay(DELAY);

    renderText(font, 0, 150, "Logging event 2: ...", renderer);
    // log event 2
    hqm_log("test_event", "{\"text\": \"sdl_test\", \"event\": \"app_start\"}");
    renderText(font, 0, 150, "Logging event 2: OK", renderer);

    SDL_Delay(DELAY);

    retryerYPos = request_user_groups(font, renderer) + 50;
}

int request_user_groups(TTF_Font *font, SDL_Renderer *renderer) {
    renderText(font, 0, 200, "Requesting group data: ...", renderer);
    int group_y_position = 250;
    renderText(font, 0, group_y_position += 50, "================================", renderer);

    // get user groups
    UserGroupData hqm_group_data = hqm_get_user_groups();
    if (hqm_group_data.userGroups != NULL) {
        if (hqm_group_data.length == 0)
            renderText(font, 0, group_y_position += 50, "empty", renderer);

        char buf[256] = {0};
        for (int i = 0; i < hqm_group_data.length; i++, hqm_group_data.userGroups++) {
            snprintf(buf, sizeof(buf), "id:\"%s\" name:\"%s\"",
                     hqm_group_data.userGroups->id,
                     hqm_group_data.userGroups->name);
            __android_log_print(ANDROID_LOG_ERROR, "HQM", "%s", buf);
            renderText(font, 0, group_y_position += 50, buf, renderer);

            SDL_Delay(DELAY);
        }
    } else {
        renderText(font, 0, group_y_position += 50, "no data", renderer);
    }

    renderText(font, 0, 200, "Requesting group data: OK", renderer);

    renderText(font, 0, group_y_position += 50, "================================", renderer);

    return group_y_position;
}

int
renderText(TTF_Font *font, int x, int y, const char *text, SDL_Renderer *renderer) {
    SDL_Color textColor = {255, 240, 0, 255};
    SDL_Surface *solid = TTF_RenderText_Solid(font, text, textColor);

    SDL_Rect dstrect;
    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = solid->w;
    dstrect.h = solid->h;

    SDL_Rect dstrect_clearer;
    dstrect_clearer.x = x;
    dstrect_clearer.y = y;
    dstrect_clearer.w = 1080;
    dstrect_clearer.h = solid->h;

    SDL_RenderFillRect(renderer, &dstrect_clearer);

    SDL_Texture *solidTexture = SDL_CreateTextureFromSurface(renderer, solid);
    SDL_RenderCopy(renderer, solidTexture, NULL, &dstrect);
    SDL_FreeSurface(solid);

    SDL_RenderPresent(renderer);

    return y;
}