/* legend.c: All heros in Kam Yung’s Stories */

#include "ui.h"
#include "scripting.h"

int main(int argc, char *argv[])
{
    if (scripting_init())
        return -1;

    if (ui_init())
        return -1;

    ui_loop();

    ui_done();
    scripting_done();

    return 0;
}

