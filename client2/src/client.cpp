//SDL2 flashing random color example
//Should work on iOS/Android/Mac/Windows/Linux

#include <iostream>
#include <SDL2/SDL.h>
#include "core.hpp"

#ifdef __EMSCRIPTEN__
#include <
#endif

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << SDL_GetError() << std::endl;
        exit(1);
    }
    SDL_Window* window = SDL_CreateWindow("SDL_Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(window == NULL){
        std::cout << SDL_GetError() << std::endl;
        exit(1);
    }
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);
    SDL_Delay(2000);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
