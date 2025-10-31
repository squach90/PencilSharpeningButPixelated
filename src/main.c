// == TODO: make a initialize() for reset all var ==

#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        return -1;
    }

    bool running = true;
    SDL_Event event;

    int actualStep = 0;

    // == Background ==
    bool backgroundVisible = true;

    // == Banner ==
    SDL_Rect bannerRect;
    bannerRect.x = 0; // from left
    bannerRect.y = 50; // from top
    bannerRect.w = 768; // 512 * 1.5
    bannerRect.h = 192; // 128 * 1.5
    bool bannerVisible = true;

    // == Sharpener == 420 × 256
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
    SDL_Surface* timerTextSurface = TTF_RenderText_Solid(font, "00 : 00 : 12", timerTextColor);
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

    while (running) {

        const uint8_t *keystate = SDL_GetKeyboardState(NULL);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (keystate[SDL_SCANCODE_UP]) {
                actualStep++;
                pencil.rect.x -= 90;
            };
        }
        // === Logic ===
        if (actualStep >= NEED_STEP) {
            SDL_DestroyTexture(pencil.texture);
            pencil = spawnPencil(state.renderer);
            actualStep = 0;
        }


        // === Render ===
        // = Image =
        if (backgroundVisible) {
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

        if (leftArrow.visible) {
            SDL_RenderCopy(state.renderer, leftArrow.texture, NULL, &leftArrow.rect);
        }

        if (rightArrow.visible) {
            SDL_RenderCopy(state.renderer, rightArrow.texture, NULL, &rightArrow.rect);
        }

        if (upArrow.visible) {
            SDL_RenderCopy(state.renderer, upArrow.texture, NULL, &upArrow.rect);
        }


        // = Text =
        if (timerTextVisible) {
            SDL_RenderCopy(state.renderer, timerTextTexture, NULL, &timerTextRect);
        }

        // == Render (real) ==
        SDL_RenderPresent(state.renderer);
    }

    // === Destroy ==
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(bannerTexture);
    SDL_DestroyTexture(pencil.texture);
    SDL_DestroyTexture(sharpenerTexture);
    SDL_DestroyTexture(leftArrow.texture);
    SDL_DestroyTexture(rightArrow.texture);
    SDL_DestroyTexture(upArrow.texture);
    SDL_FreeSurface(backgroundIMG);
    SDL_FreeSurface(sharpenerIMG);
    SDL_FreeSurface(bannerIMG);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);

}
