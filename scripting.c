/* scripting.c: Lua based scripting */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "ui.h"

lua_State *config_state;

static int lua_get_number(lua_State *L, const char *name)
{
    lua_getglobal(L, name);

    if (! lua_isnumber(L, -1))
    {
        fprintf(stderr, "'%s' should be a number.\n", name);
        return -1;
    }

    return lua_tointeger(L, -1);
}

static const char *lua_get_str(lua_State *L, const char *name)
{
    lua_getglobal(L, name);

    if (! lua_isstring(L, -1))
    {
        fprintf(stderr, "'%s' should be a string.\n", name);
        return NULL;
    }

    return lua_tostring(L, -1);
}

static int lua_set_title(lua_State *L)
{
    const char *title = lua_tostring(L, 1);

    ui_set_title(title);

    return 0;
}

static const struct luaL_Reg ui_lib[] = {
    { "set_title", lua_set_title },
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

    lua_getglobal(config_state, "init");
    if (lua_pcall(config_state, 0, 0, 0))
    {
        fprintf(stderr, "Error running function 'init': %s.\n",
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

