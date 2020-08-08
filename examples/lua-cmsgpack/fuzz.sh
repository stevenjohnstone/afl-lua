#!/bin/bash 

luarocks install --tree modules --lua-version 5.3 lua-cmsgpack 0.4.0-0 CC="afl-gcc" CFLAGS="-ggdb -fPIC"
luarocks path

export LUA_PATH="$LUA_PATH;modules/lib/lua/5.3/?.lua"
export LUA_CPATH="$LUA_CPATH;modules/lib/lua/5.3/?.so"
mkdir -p {in,out}
echo -n "\0\0" > in/first

start=$(date +%s.%N)
AFL_BENCH_UNTIL_CRASH=1 afl-fuzz -i in -o out ../../afl-lua cmsg.lua
duration=$(echo "$(date +%s.%N) - $start" | bc)

for crash in out/crashes/id*; do
	../../afl-lua -r cmsg.lua < "$crash"
done
