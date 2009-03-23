#ifndef UI_H
#define UI_H

int ui_init();
int ui_loop();
void ui_done();

void ui_set_title(const char *title);
void ui_draw_text(const char *text, int x, int y, unsigned int c);
void ui_draw_centerd_text(const char *text, int y, unsigned int c);

#endif
