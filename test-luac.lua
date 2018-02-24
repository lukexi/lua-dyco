#! ../rtlua/lua

--- FROM REALTALK
local function add_to_semicolon_list (list,entry)
    if list:match(entry .. "%f[;\0]") then return list end  -- already there
    return list .. ";" .. entry
end

function add_to_lua_cpath (path)
    package.cpath = add_to_semicolon_list(package.cpath, path)
end
--- END FROM REALTATLK

function unrequire(m)

end


local dynamic_c = require 'dynamic_c'

local c_source = [[
#include "lua.h"
#include "lauxlib.h"

int foo(lua_State *L) {
    printf("Hello from C!\n");
    return 0;
}

int luaopen_luatest (lua_State *L) {

    static const struct luaL_Reg lua_functions[] = {
        { "foo", foo },
        { NULL, NULL },
    };

    luaL_newlib(L, lua_functions);

    return 1;
}
]]

add_to_lua_cpath("/tmp/?.so")

library = dynamic_c.create_library("luatest", c_source)
luatest = require 'luatest'
luatest.foo()

print "Reloading"

local c_source_2 = [[
#include "lua.h"
#include "lauxlib.h"

int foo(lua_State *L) {
    printf("UPDATED HELLO FROM C WAHA!\n");
    return 0;
}

int luaopen_luatest (lua_State *L) {

    static const struct luaL_Reg lua_functions[] = {
        { "foo", foo },
        { NULL, NULL },
    };

    luaL_newlib(L, lua_functions);

    return 1;
}
]]

dynamic_c.update_source(library, c_source_2)
package.loaded["luatest"] = nil
luatest = require 'luatest'
luatest.foo()

