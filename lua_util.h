
extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

#include <string>

namespace Lua
{
    /* Function: InitializeLuaState
     *
     * Set up a Lua state for Darkstone. Adds Darkstone-specific functions,
     * sets the Lua path, and so forth.
     *
     * Parameters:
     *  state - The state to initialize.
     *
     * Returns:
     *  True if initialization succeeded; false otherwise.
     */
    bool InitializeLuaState(lua_State * state);

    /* Method: interpret
     *
     * Interpret an arbitrary Lua string in the given state.
     *
     * Parameters:
     *  state - The state to interpret in.
     *  code - The Lua code to interpret.
     *
     * Returns:
     *  True if the interpretation succeeded (i.e. no error was
     *  returned); false otherwise.
     */
    bool Interpret(lua_State * state, const std::string & code);

    /* Function: GetStringField
     *
     * Gets a field from a Lua table and tries to convert it to a
     * string. Assumes that the table is at the top of the Lua stack.
     * If the field does not exist, or cannot be converted to a string,
     * the function returns false. Otherwise, it returns true.
     *
     * Parameters:
     *  state  - The Lua state to act in.
     *  field  - The name of the field to query.
     *  result - The value of the field, if it could be converted to
     *           a string value.
     *
     * Returns:
     *  True if the field existed and could be converted to a string;
     *  false otherwise.
     */
    bool GetStringField(lua_State * state, const char * field, std::string & result);

    /* Function: GetStringField
     *
     * Gets a field from a Lua table and tries to convert it to an
     * integer. Assumes that the table is at the top of the Lua stack.
     * If the field does not exist, or cannot be converted to an integer,
     * the function returns false. Otherwise, it returns true.
     *
     * Parameters:
     *  state  - The Lua state to act in.
     *  field  - The name of the field to query.
     *  result - The value of the field, if it could be converted to
     *           an integer value.
     *
     * Returns:
     *  True if the field existed and could be converted to an integer;
     *  false otherwise.
     */
    bool GetIntField(lua_State * state, const char * field, int & result);
}


