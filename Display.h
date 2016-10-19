//
// Created by mhl on 19.10.16.
//

#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class Display {
public:
    Display();
    bool init(const std::string &title, int width, int height);

private:
    SDL_Window *m_window;
    SDL_Renderer *m_renderer;
    SDL_Surface *m_surface;

};


#endif //CHIP8_DISPLAY_H
