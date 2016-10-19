//
// Created by mhl on 19.10.16.
//

#include "Display.h"
#include <string>
#include <iostream>

#include <SDL2/SDL_ttf.h>

Display::Display()
    : m_window(nullptr), m_surface(nullptr) {

}

bool Display::init(const std::string &title, int width, int height) {
    if( SDL_Init( SDL_INIT_VIDEO) < 0 ) {
        std::cerr << "Unable to initialize SDL\n";
        return false;
    }

    SDL_CreateWindowAndRenderer( width, height, SDL_WINDOW_SHOWN, &m_window, &m_renderer);
    SDL_RenderPresent(m_renderer);
    if(m_window == nullptr ) {
        std::cerr << "Unable to create SDL Window";
        return false;
    }
    m_surface = SDL_GetWindowSurface( m_window );

    TTF_Init();
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 24); //this opens a font style and sets a size
    if(font == nullptr) {
        std::cerr << "Cannot load ttf\n" << TTF_GetError();
    }

    SDL_Color font_color = {255, 255, 255};
    auto surfaceMessage = TTF_RenderText_Solid(font, "Hello World", font_color);
    SDL_Texture* message = SDL_CreateTextureFromSurface(m_renderer, surfaceMessage);
    SDL_Rect message_rect; //create a rect
    message_rect.x = 0;  //controls the rect's x coordinate
    message_rect.y = 0; // controls the rect's y coordinte
    message_rect.w = 75; // controls the width of the rect
    message_rect.h = 20; // controls the height of the rect
    SDL_RenderCopy(m_renderer, message, NULL, &message_rect);

    SDL_RenderPresent(m_renderer);
    return true;
}





