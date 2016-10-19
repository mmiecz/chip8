//
// Created by mhl on 19.10.16.
//

#include "Display.h"
#include <string>
#include <iostream>

Display::Display()
    : m_window(nullptr), m_surface(nullptr) {

}

bool Display::init(const std::string &title, int width, int height) {
    if( SDL_Init( SDL_INIT_VIDEO) < 0 ) {
        std::cerr << "Unable to initialize SDL\n";
        return false;
    }

    m_window = SDL_CreateWindow(title.c_str(), 500, 500, width, height, SDL_WINDOW_SHOWN);
    if(m_window == nullptr ) {
        std::cerr << "Unable to create SDL Window";
        return false;
    }
    m_surface = SDL_GetWindowSurface( m_window );
    return true;
}





