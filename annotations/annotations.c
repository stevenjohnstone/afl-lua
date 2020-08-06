/******************************************************************************
* Copyright (C) 2020 Steven Johnstone
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

/******************************************************************************
* Implementation of some annotation primitives descrived here:
* https://github.com/RUB-SysSec/ijon
* 
* Implemented IJON_MAX and IJON_MIN without requiring modifications to afl-fuzz.
* Max and min values are stored locally and when one changes a value in the
* shared memory is modified to indicate that a new state has been reached.
******************************************************************************/
 

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

int luaopen_annotations(lua_State *);

extern unsigned char *__afl_global_area_ptr;
extern unsigned int __afl_state;
extern unsigned int __afl_mask;
extern const size_t afl_shm_size;

static unsigned int __afl_state_log;
static unsigned int minmax_area[1<<8];
static size_t minmax_area_size = sizeof(minmax_area);


static int afl_map_set(lua_State *L) {
    int val = lua_tointeger(L, 1);
    __afl_global_area_ptr[__afl_state ^ (val % afl_shm_size)] |= 1;
    return 0;
}

static int afl_map_inc(lua_State *L) {
    int val = lua_tointeger(L, 1);
    __afl_global_area_ptr[__afl_state ^ (val % afl_shm_size)] += 1;
    return 0;
}

static int afl_state_xor(lua_State *L) {
    int val = lua_tointeger(L, 1);
    __afl_state = (__afl_state ^ val) % afl_shm_size;
    return 0;
}

static int afl_state_push(lua_State *L) {
    int val = lua_tointeger(L, 1);
    __afl_state = (__afl_state ^ __afl_state_log) % afl_shm_size;
    __afl_state_log = (__afl_state_log << 8) | (val & 0xff);
    __afl_state = (__afl_state ^ __afl_state_log) % afl_shm_size;
    return 0;
}

static int afl_disable(__attribute__((unused)) lua_State *L) {
    __afl_mask = 0;
    return 0;
}

static int afl_enable(__attribute__((unused)) lua_State *L) {
    __afl_mask = ~((unsigned int)0);
    return 0;
}

static void max(unsigned int slot, unsigned int value) {
    if (minmax_area[slot%minmax_area_size] < value) {
        const unsigned int offset = ((value << 8) | slot)%afl_shm_size;
        minmax_area[slot%minmax_area_size] = value;
        __afl_global_area_ptr[__afl_state ^ offset] |= 1;
    }
}

static int afl_max(lua_State *L) {
    unsigned int slot    = lua_tointeger(L, 1);
    unsigned int value   = lua_tointeger(L, 2);
    max(slot, value);
    return 0;
}

static int afl_min(lua_State *L) {
    unsigned int slot    = lua_tointeger(L, 1);
    unsigned int value   = lua_tointeger(L, 2);
    max(slot, (~(unsigned int)0) - value);
    return 0;
}


static const struct luaL_Reg annotations[] = {
    {"afl_map_set", afl_map_set},
    {"afl_map_inc", afl_map_inc},
    {"afl_state_xor", afl_state_xor},
    {"afl_state_push", afl_state_push},
    {"afl_disable", afl_disable},
    {"afl_enabled", afl_enable},
    {"afl_min", afl_min},
    {"afl_max", afl_max},
    {NULL, NULL}
};

int luaopen_annotations(lua_State *L) {
    luaL_newlib(L, annotations);
    return 1;
}