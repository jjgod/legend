/* ui.c */

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "ui.h"
#include "config.h"
#include "scripting.h"

SDL_Surface *screen;
TTF_Font *chinese_font, *english_font;

struct fade_effect {
    SDL_TimerID timer;

    int alpha;
    int max;
    int step;

    int x;
    int y;

    SDL_Surface *src;
    SDL_Surface *dest;
};

static TTF_Font *get_font(const char *name)
{
    TTF_Font *font = TTF_OpenFont(config_get_str_from_table(name, "path"),
                                  config_get_int_from_table(name, "size"));

    if (! font)
    {
        fprintf(stderr, "Failed to find font: %s\n", SDL_GetError());
        exit(-1);
    }

    return font;
}

static void render_text(SDL_Surface *sur,
                        TTF_Font *font,
                        const char *text,
                        int x, int y,
                        SDL_Color color)
{
    SDL_Surface *textSur = TTF_RenderUTF8_Blended(font, text, color);

    SDL_Rect dest = { x, y, textSur->w, textSur->h };

    SDL_FillRect(sur, &dest, 0);
    SDL_BlitSurface(textSur, NULL, sur, &dest);

    SDL_FreeSurface(textSur);
}

#define INVALID 0x80000000

#define get(c)  c = *strptr++; \
    if (chars) (*chars)++; \
    if ((c) == 0) return (unsigned int)EOF

unsigned int sgetu8(unsigned char *strptr, int *chars)
{
    unsigned int c;
    int i, iterations;
    unsigned char ch;

    if (chars) *chars = 0;

    if (strptr == NULL)
        return (unsigned int)EOF;

    get(c);

    if ((c & 0xFE) == 0xFC) {
        c &= 0x01;
        iterations = 5;
    }
    else if ((c & 0xFC) == 0xF8) {
        c &= 0x03;
        iterations = 4;
    }
    else if ((c & 0xF8) == 0xF0) {
        c &= 0x07;
        iterations = 3;
    }
    else if ((c & 0xF0) == 0xE0) {
        c &= 0x0F;
        iterations = 2;
    }
    else if ((c & 0xE0) == 0xC0) {
        c &= 0x1F;
        iterations = 1;
    }
    else if ((c & 0x80) == 0x80)
        return INVALID;
    else return c;

    for (i = 0; i < iterations; i++) {
        get(ch);
        if ((ch & 0xC0) != 0x80)
            return INVALID;
        c <<= 6;
        c |= ch & 0x3F;
    }

    return c;
}

enum lang_state {
    STATE_ENGLISH, STATE_CHINESE
};

enum lang_state is_chinese(unsigned int x)
{
    // Punct & Radicals
    if (x >= 0x2e80 && x <= 0x33ff)
        return 1;

    // Fullwidth Latin Characters
    if (x >= 0xff00 && x <= 0xffef)
        return 1;

    // CJK Unified Ideographs &
    // CJK Unified Ideographs Extension A
    if (x >= 0x4e00 && x <= 0x9fbb)
        return 1;

    // CJK Compatibility Ideographs
    if (x >= 0xf900 && x <= 0xfad9)
        return 1;

    // CJK Unified Ideographs Extension B
    if (x >= 0x20000 && x <= 0x2a6d6)
        return 1;

    // CJK Compatibility Supplement
    if (x >= 0x2f8000 && x <= 0x2fa1d)
        return 1;

    return 0;
}

#define MAX_CHAR_BUF    1024

static int draw_part_text(SDL_Surface *sur,
                          int state,
                          const char *text,
                          int len,
                          int x, int y,
                          SDL_Color color)
{
    char buf[MAX_CHAR_BUF];
    TTF_Font *font = state ? chinese_font : english_font;
    int w = 0, h = 0;

    if (len >= MAX_CHAR_BUF || len <= 0)
        return 0;

    strncpy(buf, text, len);
    buf[len] = 0;

    render_text(sur, font, buf, x, y, color);
    TTF_SizeUTF8(font, buf, &w, &h);

    fprintf(stderr, "render_text(%s, %s, %d, %d, [%d, %d, %d])\n",
            state == STATE_CHINESE ? "ch" : "en", buf, x, y,
            color.r, color.g, color.b);

    return w;
}

static void draw_text(SDL_Surface *sur,
                      const char *text,
                      int x, int y,
                      SDL_Color color)
{
    int len = strlen(text);
    int state = STATE_ENGLISH, start, i, total, chars;

    for (i = 0, total = 0, start = 0, chars = 0;
         i < len; total++, i += chars)
    {
        int newstate;
        unsigned int ch = sgetu8((unsigned char *) (text + i), &chars);

        if (ch == EOF)
            break;

        newstate = is_chinese(ch);
        if (i == 0)
            state = newstate;

        if (state != newstate && i - 1 > start)
        {
            x += draw_part_text(sur, state,
                                text + start,
                                i - start,
                                x, y, color);
            start = i;
            state = newstate;
            total = 0;
        }
    }

    // There may be some remaining text
    draw_part_text(sur, state, text + start,
                   i - start, x, y, color);
}

static int text_width_in_state(int state, const char *text, int len)
{
    char buf[MAX_CHAR_BUF];
    TTF_Font *font = state ? chinese_font : english_font;
    int w, h;

    if (len >= MAX_CHAR_BUF || len <= 0)
        return 0;

    strncpy(buf, text, len);
    buf[len] = 0;

    TTF_SizeUTF8(font, buf, &w, &h);
    return w;
}

static int text_width(const char *text)
{
    int len = strlen(text);
    int state = STATE_ENGLISH, start, i, total, chars, width = 0;

    for (i = 0, total = 0, start = 0, chars = 0;
         i < len; total++, i += chars)
    {
        int newstate;
        unsigned int ch = sgetu8((unsigned char *) (text + i), &chars);

        if (ch == EOF)
            break;

        newstate = is_chinese(ch);
        if (i == 0)
            state = newstate;

        if (state != newstate && i - 1 > start)
        {
            width += text_width_in_state(state, text + start, i - start);
            start = i;
            state = newstate;
            total = 0;
        }
    }

    width += text_width_in_state(state, text + start, i - start);
    return width;
}

int font_init()
{
    TTF_Init();

    chinese_font = get_font("chinese_font");
    english_font = get_font("english_font");

    return chinese_font && english_font;
}

void font_done()
{
    TTF_CloseFont(chinese_font);
    TTF_CloseFont(english_font);
    TTF_Quit();
}

int ui_init()
{
    int width, height;

    font_init();

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

    scripting_init_ui();
    SDL_Flip(screen);

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
    font_done();
    SDL_Quit();
}

void ui_set_title(const char *title)
{
    SDL_WM_SetCaption(title, NULL);
}

static SDL_Color color_from_int(unsigned int c)
{
    SDL_Color color;

    color.r = c >> 16 & 0xFF;
    color.g = c >> 8 & 0xFF;
    color.b = c & 0xFF;

    return color;
}

void ui_draw_text(const char *text, int x, int y, unsigned int c)
{
    draw_text(screen, text, x, y, color_from_int(c));
}

void ui_draw_centerd_text(const char *text, int y, unsigned int c)
{
    int w, x;

    w = text_width(text);
    x = (screen->w - w) / 2;

    draw_text(screen, text, x, y, color_from_int(c));
}

int ui_show_image(const char *path, int x, int y, int flags)
{
    SDL_Surface *image = IMG_Load(path);

    if (! image)
        return -1;

    if (flags & UI_SHOW_IMAGE_CENTERED)
        x = (screen->w - image->w) / 2;

    SDL_Rect dest = { x, y, image->w, image->h };

    if (flags & UI_SHOW_IMAGE_FADE_IN)
    {
        int i;

        for (i = 0; i < 35; i++)
        {
            SDL_Delay(15);
            SDL_SetAlpha(image, SDL_SRCALPHA, i);
            SDL_BlitSurface(image, NULL, screen, &dest);
            SDL_Flip(screen);
        }
    }
    else
    {
        SDL_BlitSurface(image, NULL, screen, &dest);
        SDL_Flip(screen);
    }

    return 0;
}

