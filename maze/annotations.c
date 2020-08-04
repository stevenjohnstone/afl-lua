#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

extern unsigned char *__afl_global_area_ptr;
extern unsigned int __afl_state;
extern const size_t afl_shm_size;


static int afl_set(lua_State *L) {
    const int row = lua_tointeger(L, 1);
    const int col = lua_tointeger(L, 2);
    const int offset = (row << 8 | col)%afl_shm_size;
    __afl_global_area_ptr[__afl_state ^ offset] |= 1;
    return 0;
}

static const struct luaL_Reg annotations[] = {
    {"afl_set", afl_set},
    {NULL, NULL}
};

int luaopen_annotations(lua_State *L) {
    luaL_newlib(L, annotations);
    return 1;
}