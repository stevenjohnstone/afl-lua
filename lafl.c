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

 

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

extern unsigned char *__afl_global_area_ptr;
extern unsigned int __afl_prev_loc;
extern unsigned int __afl_state;
extern unsigned int __afl_enabled;
extern const size_t __afl_shm_size;
extern unsigned int *__afl_scratch_area;
extern const size_t __afl_scratch_area_size;
extern unsigned int __afl_state_log;


static int afl_map_set(lua_State *L) {
    int val = lua_tointeger(L, 1);
    __afl_global_area_ptr[__afl_state ^ (val % __afl_shm_size)] |= 1;
    return 0;
}

static int afl_map_inc(lua_State *L) {
    int val = lua_tointeger(L, 1);
    __afl_global_area_ptr[__afl_state ^ (val % __afl_shm_size)] += 1;
    return 0;
}

static int afl_state_xor(lua_State *L) {
    int val = lua_tointeger(L, 1);
    __afl_state = (__afl_state ^ val) % __afl_shm_size;
    return 0;
}

static int afl_state_push(lua_State *L) {
    int val = lua_tointeger(L, 1);
    __afl_state = (__afl_state ^ __afl_state_log) % __afl_shm_size;
    __afl_state_log = (__afl_state_log << 8) | (val & 0xff);
    __afl_state = (__afl_state ^ __afl_state_log) % __afl_shm_size;
    return 0;
}

static int afl_disable(__attribute__((unused)) lua_State *L) {
    __afl_enabled = 0;
    return 0;
}

static int afl_enable(__attribute__((unused)) lua_State *L) {
    __afl_enabled = 1;
    __afl_prev_loc = 0;
    return 0;
}

static void max(unsigned int slot, unsigned int value) {
    slot = slot %(__afl_scratch_area_size/sizeof(__afl_scratch_area[0]));
    if (__afl_scratch_area[slot] < value) {
        const unsigned int offset = ((value << 8) | slot)%__afl_shm_size;
        __afl_scratch_area[slot] = value;
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

static int afl_loop_iter(lua_State *L) {
    lua_Integer max = lua_tointeger(L, 1);
    lua_Integer n = lua_tointeger(L, 2);
    if (n >= max) {
        __afl_enabled = 0;
        return 0;
    }
    if (n > 0) {
        raise(SIGSTOP);
    }

    __afl_enabled = 1;
    if (n == 0) {
        memset(__afl_global_area_ptr, 0, __afl_shm_size);
    }
    __afl_global_area_ptr[0] = 1;
    __afl_prev_loc = 0;
    memset(__afl_scratch_area, 0, __afl_scratch_area_size);
    lua_pushinteger(L, n + 1);
    return 1;
}

static int afl_loop(lua_State *L) {
    luaL_checkinteger(L, 1);
    lua_pushcfunction(L, afl_loop_iter);
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 0);
    return 3;
}


static const struct luaL_Reg afl[] = {
/******************************************************************************
* Implementation of some annotation primitives described here:
* https://github.com/RUB-SysSec/ijon
* 
* Implemented IJON_MAX and IJON_MIN without requiring modifications to afl-fuzz.
* Max and min values are stored in the scratch area. When a value changes a state
* change is reported to afl via the shared memory bitmap.
******************************************************************************/
    {"min", afl_min},
    {"max", afl_max},
    {"map_set", afl_map_set},
    {"map_inc", afl_map_inc},
    {"state_xor", afl_state_xor},
    {"state_push", afl_state_push},
/* IJON primitives end */
    {"disable", afl_disable},
    {"enable", afl_enable},
    {"loop", afl_loop},
    {NULL, NULL}
};

LUAMOD_API int luaopen_afl(lua_State *L) {
    luaL_newlib(L, afl);
    return 1;
}