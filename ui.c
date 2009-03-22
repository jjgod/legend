/* ui.c */

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "config.h"

SDL_Surface *screen;

int ui_init()
{
    int width, height;

    width = config_get_int("width");
    height = config_get_int("height");

    SDL_putenv("SDL_VIDEO_WINDOW_POS=center");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return -1;
    }

    atexit(SDL_Quit);

    screen = SDL_SetVideoMode(width, height, 32,
                              SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen == NULL)
    {
        fprintf(stderr, "Unable to set %dx%d video: %s\n",
                width, height, SDL_GetError());
        return -1;
    }

    return 0;
}

int ui_loop()
{
    SDL_Event event;
    int done = 0;

    while (! done && SDL_WaitEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            // Quit when user presses a key.
            done = 1;
            break;

        case SDL_QUIT:
            done = 1;
            break;

        default:
            break;
        }
    }

    return 0;
}

void ui_done()
{
    SDL_Quit();
}

void ui_set_title(const char *title)
{
    SDL_WM_SetCaption(title, NULL);
}

