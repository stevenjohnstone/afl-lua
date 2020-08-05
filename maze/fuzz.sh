#!/bin/bash -e
mkdir -p {in,out}
echo -n "s" > in/first

gcc -I ../ -shared -fPIC annotations.c -o annotations.so
start=$(date +%s.%N)
AFL_BENCH_UNTIL_CRASH=1 afl-fuzz -i in -o out ../afl-lua mazefuzz.lua
duration=$(echo "$(date +%s.%N) - $start" | bc)

for crash in out/crashes/id*; do
	lua replay.lua < "$crash"
done

echo "Solution took $duration seconds to find"
