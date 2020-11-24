
#include "lua_util.h"
#include "globals.h"
#include "paths.const.h"
#include "World.h"

bool Lua::InitializeLuaState(lua_State * state)
{
    // Set up the Lua path
    if (!Interpret(state,"package.path = package.path .. \";"LUA_DIR"?.lua;"
                   LUA_DIR"?\""))
    {
        gTheWorld->LogBugString("Couldn't set Lua path!");
        return false;
    }

    // Load up the serialization library
    if (!Interpret(state, "require 'serialize'"))
    {
        gTheWorld->LogBugString("Couldn't load serialization module!");
        return false;
    }
    
    return true;
}

bool Lua::Interpret(lua_State * state, const std::string & code)
{
    int retval = luaL_loadstring(state, code.c_str())
        || lua_pcall(state, 0, 0, 0);

    if (retval != 0)
    {
        gTheWorld->LogBugString("Error interpreting code:");
        gTheWorld->LogBugString(lua_tostring(state, -1));
        lua_pop(state, 1); // pop the error message
        return false;
    }

    return true;
}

bool Lua::GetStringField(lua_State * state, const char * field,
                         std::string & result)
{
    lua_getfield(state, -1, field);

    if ( lua_isnil(state, -1) || !lua_isstring(state, -1) ) {
        lua_pop(state, 1); // pop whatever it was
        return false;
    }

    result = lua_tostring(state, -1);
    // remove the string from the state
    lua_pop(state, 1);

    return true;
}

bool Lua::GetIntField(lua_State * state, const char * field,
                         int & result)
{
    lua_getfield(state, -1, field);

    if ( lua_isnil(state, -1) || !lua_isnumber(state, -1) ) {
        lua_pop(state, 1); // pop whatever it was
        return false;
    }

    result = (int) lua_tonumber(state, -1);
    // remove the int from the state
    lua_pop(state, 1);

    return true;
}

