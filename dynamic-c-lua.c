#include "lua.h"
#include "lauxlib.h"
#include "dynamic.h"

int create_library(lua_State *L) {
    const char* Name   = lua_tostring(L, 1);
    const char* Source = lua_tostring(L, 2);
    library* Library = CreateLibrary((char*)Name, (char*)Source);
    lua_pushlightuserdata(L, (void *)Library);
    return 1;
}

int update_source(lua_State *L) {
    library* Library = lua_touserdata(L, 1);
    const char* Source = lua_tostring(L, 2);
    int Result = UpdateLibrarySource(Library, (char*)Source);
    lua_pushnumber(L, Result);
    return 1;
}

int luaopen_dynamic_c (lua_State *L) {

    static const struct luaL_Reg lua_functions[] = {
        { "create_library", create_library },
        { "update_source",  update_source  },
        { NULL, NULL },
    };

    luaL_newlib(L, lua_functions);

    return 1;
}
