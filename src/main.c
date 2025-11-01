// == TODO: make a initialize() for reset all var ==

#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_scancode.h"
#include "SDL_stdinc.h"
#include "SDL_surface.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define NEED_STEP 10

struct State {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} state;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
    bool visible;
} Pencil;

typedef struct {
    SDL_Texture* texture;
    SDL_Rect rect;
    bool visible;
} Arrow;


void createWindow() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    state.window = SDL_CreateWindow("Pencil Sharpening Championship",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);

    state.renderer = SDL_CreateRenderer(state.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

Pencil spawnPencil(SDL_Renderer* renderer) {
    Pencil p; // alias

    SDL_Surface* img = IMG_Load("assets/pencil.png");
    if (!img) {
        printf("Error loading pencil: %s\n", SDL_GetError());
        p.texture = NULL;
        return p;
    }

    p.texture = SDL_CreateTextureFromSurface(renderer, img);
    SDL_FreeSurface(img);

    // = Initial Var =
    p.rect.x = 1000;
    p.rect.y = 650;
    p.rect.w = 780;
    p.rect.h = 192;

    p.visible = true;

    return p;
}

Arrow spawnArrow(SDL_Renderer* renderer, const char* path, int x, int y, int w, int h) {
    Arrow arrow;

    SDL_Surface* img = IMG_Load(path);
    if (!img) {
        printf("Error loading arrow image (%s): %s\n", path, SDL_GetError());
        arrow.texture = NULL;
        arrow.visible = false;
        return arrow;
    }

    arrow.texture = SDL_CreateTextureFromSurface(renderer, img);
    SDL_FreeSurface(img);

    arrow.rect.x = x;
    arrow.rect.y = y;
    arrow.rect.w = w;
    arrow.rect.h = h;
    arrow.visible = true;

    return arrow;
}


int main(void) {
    createWindow();
    srand(time( NULL )); // Random Seed

    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        return -1;
    }

    bool running = true;
    SDL_Event event;

    int actualStep = 0;
    int score = 0;

    // == Background ==
    bool backgroundVisible = true;

    // == Banner ==
    SDL_Rect bannerRect;
    bannerRect.x = 0; // from left
    bannerRect.y = 50; // from top
    bannerRect.w = 768; // 512 * 1.5
    bannerRect.h = 192; // 128 * 1.5
    bool bannerVisible = true;

    // == Main Screen ==
    SDL_Rect mainScreenRect;
    mainScreenRect.x = 0;
    mainScreenRect.y = 0;
    mainScreenRect.w = 1920;
    mainScreenRect.h = 1080;
    bool mainScreenVisible = true;

    // == Sharpener ==
    SDL_Rect sharpenerRect;
    sharpenerRect.x = 200;
    sharpenerRect.y = 600;
    sharpenerRect.w = 504; // 420 * 1.2
    sharpenerRect.h = 307; // 256 * 1.2
    bool sharpenerVisible = true;

    Arrow leftArrow  = spawnArrow(state.renderer, "assets/LeftArrow.png",  250, 447, 165, 165); // 110 * 1.5
    Arrow rightArrow = spawnArrow(state.renderer, "assets/RightArrow.png", 500, 440, 165, 165); // 110 * 1.5
    Arrow upArrow    = spawnArrow(state.renderer, "assets/UpArrow.png",    375, 440, 165, 165); // 110 * 1.5


    // == Timer Text ==
    bool timerTextVisible = true;

    // == Font ==
    TTF_Font* font = TTF_OpenFont("assets/pixelFont.ttf", 96);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Color timerTextColor = {0, 0, 0, 0}; // black
    SDL_Surface* timerTextSurface = TTF_RenderText_Solid(font, "00:00:000", timerTextColor);
    SDL_Texture* timerTextTexture = SDL_CreateTextureFromSurface(state.renderer, timerTextSurface);

    SDL_Rect timerTextRect = {150, 90, timerTextSurface->w, timerTextSurface->h};

    // === Load Images ===
    // = Background =
    SDL_Surface* backgroundIMG = IMG_Load("assets/background.png"); // Surface -> in CPU Memory
    if(!backgroundIMG) {
        printf("Error while loading Background Image: %s",SDL_GetError());
        return -1;
    }

    SDL_Texture* backgroundTexture = SDL_CreateTextureFromSurface(state.renderer,backgroundIMG);  // Texture -> in GPU Memory

    // = Banner =
    SDL_Surface* bannerIMG = IMG_Load("assets/banner_w:_clock.png");
    if(!bannerIMG) {
        printf("Error while loading Banner Image: %s",SDL_GetError());
        return -1;
    }

    SDL_Texture* bannerTexture = SDL_CreateTextureFromSurface(state.renderer,bannerIMG);

    // = Main Screen =
    SDL_Surface* mainScreenIMG = IMG_Load("assets/MainScreen.png");
    if(!mainScreenIMG) {
        printf("Error while loading Main Screen Image: %s",SDL_GetError());
        return -1;
    }

    SDL_Texture* mainScreenTexture = SDL_CreateTextureFromSurface(state.renderer,mainScreenIMG);

    // = Pencil =
    Pencil pencil = spawnPencil(state.renderer);
    actualStep = 0;


    // = Sharpener =
    SDL_Surface* sharpenerIMG = IMG_Load("assets/sharpener.png");
    if(!sharpenerIMG) {
        printf("Error while loading Sharpener Image: %s",SDL_GetError());
        return -1;
    }

    SDL_Texture* sharpenerTexture = SDL_CreateTextureFromSurface(state.renderer,sharpenerIMG);

    int randNum = rand() % 3; // arrow chooser

    Uint32 startTime = 0;
    int countdown = 10; // Countdown in sec

    while (running) {

        // === Timer ===
        if (!mainScreenVisible) {
            Uint32 currentTime = SDL_GetTicks();
            Uint32 elapsed = (currentTime - startTime) / 1000;

            int remaining = countdown - elapsed;
            if (remaining < 0) remaining = 0;

            int minutes = remaining / 60;
            int seconds = remaining % 60;
            int ms = (currentTime - startTime) % 1000;


            // == Timer Text ==
            char timerText[32];
            sprintf(timerText, "%02d:%02d:%03d", minutes, seconds, ms); // text formating :  00:00:000

            SDL_FreeSurface(timerTextSurface);
            SDL_DestroyTexture(timerTextTexture);

            timerTextSurface = TTF_RenderText_Solid(font, timerText, timerTextColor);
            timerTextTexture = SDL_CreateTextureFromSurface(state.renderer, timerTextSurface);
            timerTextRect.w = timerTextSurface->w;
            timerTextRect.h = timerTextSurface->h;
            if (remaining == 0) {
                printf("Score: %d", score);
                running = false;
            }
        }

        const uint8_t *keystate = SDL_GetKeyboardState(NULL);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if ((randNum == 0 && !mainScreenVisible && keystate[SDL_SCANCODE_LEFT]) || (randNum == 1 && keystate[SDL_SCANCODE_UP]) || (randNum == 2 && keystate[SDL_SCANCODE_RIGHT])) {
                actualStep++;
                pencil.rect.x -= 86; // to not go after the sharpener

                randNum = rand() % 3;

                leftArrow.visible  = (randNum == 0);
                upArrow.visible    = (randNum == 1);
                rightArrow.visible = (randNum == 2);
            }
            if (keystate[SDL_SCANCODE_RETURN] && mainScreenVisible) {
                // === READY ===
                SDL_Surface* readySurface = TTF_RenderText_Solid(font, "READY...", timerTextColor);
                SDL_Texture* readyTexture = SDL_CreateTextureFromSurface(state.renderer, readySurface);
                SDL_Rect readyRect = {SCREEN_WIDTH/2 - readySurface->w/2, SCREEN_HEIGHT/2 - readySurface->h/2, readySurface->w, readySurface->h};

                SDL_RenderClear(state.renderer);
                SDL_RenderCopy(state.renderer, backgroundTexture, NULL, NULL);
                SDL_RenderCopy(state.renderer, readyTexture, NULL, &readyRect);
                SDL_RenderPresent(state.renderer);
                SDL_Delay(1000);

                // === GO! ===
                SDL_Surface* goSurface = TTF_RenderText_Solid(font, "GO!", timerTextColor);
                SDL_Texture* goTexture = SDL_CreateTextureFromSurface(state.renderer, goSurface);
                SDL_Rect goRect = {SCREEN_WIDTH/2 - goSurface->w/2, SCREEN_HEIGHT/2 - goSurface->h/2, goSurface->w, goSurface->h};

                SDL_RenderClear(state.renderer);
                SDL_RenderCopy(state.renderer, backgroundTexture, NULL, NULL);
                SDL_RenderCopy(state.renderer, goTexture, NULL, &goRect);
                SDL_RenderPresent(state.renderer);
                SDL_Delay(500);

                SDL_FreeSurface(readySurface);
                SDL_FreeSurface(goSurface);
                SDL_DestroyTexture(readyTexture);
                SDL_DestroyTexture(goTexture);

                // === Start the game ===
                mainScreenVisible = false;
                startTime = SDL_GetTicks();
            }

        }
        // === Logic ===
        if (actualStep >= NEED_STEP) {
            SDL_DestroyTexture(pencil.texture);
            pencil = spawnPencil(state.renderer);
            actualStep = 0;
            score++;
        }


        leftArrow.visible = (randNum == 0);
        upArrow.visible = (randNum == 1);
        rightArrow.visible = (randNum == 2);
        // === Render ===
        SDL_RenderClear(state.renderer);

        // = Image =
        if (backgroundVisible && !mainScreenVisible) {
            SDL_RenderCopy(state.renderer, backgroundTexture, NULL, NULL);
        }
        if (bannerVisible) {
            SDL_RenderCopy(state.renderer, bannerTexture, NULL, &bannerRect);
        }

        if (pencil.visible) {
            SDL_RenderCopy(state.renderer, pencil.texture, NULL, &pencil.rect);
        }

        if (sharpenerVisible) {
            SDL_RenderCopy(state.renderer, sharpenerTexture, NULL, &sharpenerRect);
        }

        if (leftArrow.visible && !mainScreenVisible) {
            SDL_RenderCopy(state.renderer, leftArrow.texture, NULL, &leftArrow.rect);
        }

        if (rightArrow.visible && !mainScreenVisible) {
            SDL_RenderCopy(state.renderer, rightArrow.texture, NULL, &rightArrow.rect);
        }

        if (upArrow.visible && !mainScreenVisible) {
            SDL_RenderCopy(state.renderer, upArrow.texture, NULL, &upArrow.rect);
        }

        if (mainScreenVisible) {
            SDL_RenderCopy(state.renderer, mainScreenTexture, NULL, &mainScreenRect);
        }

        // = Text =
        if (timerTextVisible && !mainScreenVisible) {
            SDL_RenderCopy(state.renderer, timerTextTexture, NULL, &timerTextRect);
        }

        // == Render (real) ==
        SDL_RenderPresent(state.renderer);
    }

    // === Destroy ===
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(bannerTexture);
    SDL_DestroyTexture(pencil.texture);
    SDL_DestroyTexture(sharpenerTexture);
    SDL_DestroyTexture(leftArrow.texture);
    SDL_DestroyTexture(rightArrow.texture);
    SDL_DestroyTexture(upArrow.texture);
    SDL_DestroyTexture(timerTextTexture);
    SDL_DestroyTexture(mainScreenTexture);
    SDL_FreeSurface(backgroundIMG);
    SDL_FreeSurface(sharpenerIMG);
    SDL_FreeSurface(bannerIMG);
    SDL_FreeSurface(mainScreenIMG);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);

}
