#!/bin/bash -e
mkdir -p {in,out}
echo -n "s" > in/first

start=$(date +%s.%N)
AFL_BENCH_UNTIL_CRASH=1 afl-fuzz -i in -o out -d ../../afl-lua obstaclefuzz.lua
duration=$(echo "$(date +%s.%N) - $start" | bc)

for crash in out/crashes/id*; do
	lua replay.lua < "$crash"
	break
done

echo "Solution took $duration seconds to find"
