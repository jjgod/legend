/* scripting.c: Lua based scripting */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "ui.h"

lua_State *config_state;

static int lua_get_table(lua_State *L, const char *name)
{
    lua_getglobal(L, name);
    if (! lua_istable(L, -1))
        return -1;

    return 0;
}

static int number_from_stack(lua_State *L)
{
    if (! lua_isnumber(L, -1))
        return -1;

    int ret = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return ret;
}

static const char *str_from_stack(lua_State *L)
{
    if (! lua_isstring(L, -1))
        return NULL;

    const char *ret = lua_tostring(L, -1);
    lua_pop(L, 1);
    return ret;
}

static int lua_get_number(lua_State *L, const char *name)
{
    lua_getglobal(L, name);

    return number_from_stack(L);
}

static const char *lua_get_str(lua_State *L, const char *name)
{
    lua_getglobal(L, name);

    return str_from_stack(L);
}

static int lua_set_title(lua_State *L)
{
    const char *title = lua_tostring(L, 1);

    ui_set_title(title);

    return 0;
}

static int lua_draw_text(lua_State *L)
{
    const char *text = lua_tostring(L, 1);
    int x, y;
    unsigned int color;

    x = lua_tointeger(L, 2);
    y = lua_tointeger(L, 3);
    color = lua_tointeger(L, 4);

    ui_draw_text(text, x, y, color);

    return 0;
}

static int lua_draw_centerd_text(lua_State *L)
{
    const char *text = lua_tostring(L, 1);
    int y;
    unsigned int color;

    y = lua_tointeger(L, 2);
    color = lua_tointeger(L, 3);

    ui_draw_centerd_text(text, y, color);

    return 0;
}

static const struct luaL_Reg ui_lib[] = {
    { "set_title",         lua_set_title },
    { "draw_text",         lua_draw_text },
    { "draw_centerd_text", lua_draw_centerd_text },
    { NULL, NULL },
};

int scripting_init()
{
    config_state = luaL_newstate();
    luaL_register(config_state, "ui", ui_lib);

    if (luaL_loadfile(config_state, "config.lua") ||
        lua_pcall(config_state, 0, 0, 0))
    {
        fprintf(stderr, "Cannot run %s.\n",
                lua_tostring(config_state, -1));
        return -1;
    }

    return 0;
}

int scripting_init_ui()
{
    lua_getglobal(config_state, "init_ui");
    if (lua_pcall(config_state, 0, 0, 0))
    {
        fprintf(stderr, "Error running function 'init_ui': %s.\n",
                lua_tostring(config_state, -1));
        return -1;
    }

    return 0;
}

void scripting_done()
{
    lua_close(config_state);
}

int config_get_int(const char *name)
{
    return lua_get_number(config_state, name);
}

const char *config_get_str(const char *name)
{
    return lua_get_str(config_state, name);
}

int config_get_int_from_table(const char *table, const char *name)
{
    if (lua_get_table(config_state, table))
        return -1;

    lua_getfield(config_state, -1, name);

    int ret = number_from_stack(config_state);
    lua_pop(config_state, -1);
    return ret;
}

const char *config_get_str_from_table(const char *table, const char *name)
{
    if (lua_get_table(config_state, table))
        return NULL;

    lua_getfield(config_state, -1, name);

    const char *str = str_from_stack(config_state);
    lua_pop(config_state, -1);
    return str;
}

