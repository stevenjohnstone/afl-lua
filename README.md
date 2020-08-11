# afl-lua
Fork of Lua adding AFL (https://github.com/google/afl) instrumentation to allow Lua scripts (not the VM itself) to be fuzzed.

1. [Building](#Building)
2. [Try it out](#Tryitout)
3. [C module fuzzing](#Cmodulefuzzing)
4. [Other Approaches](#OtherApproaches)
5. [Advanced Usage](#AdvancedUsage)
	* 5.1. [Annotations](#Annotations)
		* 5.1.1. [Solving a Maze](#SolvingaMaze)
		* 5.1.2. [Obstacle Course](#ObstacleCourse)
    * 5.2  [Persistent Mode](#PersistentMode)


##  1. <a name='Building'></a>Building

On Linux (maybe other POSIX-y systems?), 

```
make
```

In the directory, there'll be an executable called "afl-lua".

##  2. <a name='Tryitout'></a>Try it out

With afl-fuzz installed, make a test case called "test.lua" containing the following:

```lua
local function fuzz()
    local data = io.read("*all")
    if #data > 2 then
        local c = data:sub(1,1)
        local d = data:sub(2,2)
        local idx = tonumber(c)
        local idx2 = tonumber(d)
        local target = 50
        if idx and idx2 then
            for i = 1, idx*idx2 do
                if i == target then
                    if #data > 10 then
                        if data:sub(5,5) == 'f' then
                            if #data == 11 then
                                error('foo')
                            end
                        end
                    end
                end
            end
        end
    end
end

fuzz()
```

Run afl-fuzz with

```
mkdir {in, out}
echo foo > in/foo # just something to get the fuzzer corpus started
afl-fuzz -i in -o out ./afl-lua test.lua
```

You should see the fuzzer kick off and in short order find an input with will cause the script to exit on an error:

```
                       american fuzzy lop 2.52b (afl-lua)

┌─ process timing ─────────────────────────────────────┬─ overall results ─────┐
│        run time : 0 days, 0 hrs, 0 min, 22 sec       │  cycles done : 5      │
│   last new path : 0 days, 0 hrs, 0 min, 5 sec        │  total paths : 31     │
│ last uniq crash : 0 days, 0 hrs, 0 min, 4 sec        │ uniq crashes : 1      │
│  last uniq hang : none seen yet                      │   uniq hangs : 0      │
├─ cycle progress ────────────────────┬─ map coverage ─┴───────────────────────┤
│  now processing : 26* (83.87%)      │    map density : 0.06% / 0.22%         │
│ paths timed out : 0 (0.00%)         │ count coverage : 1.15 bits/tuple       │
├─ stage progress ────────────────────┼─ findings in depth ────────────────────┤
│  now trying : bitflip 4/1           │ favored paths : 7 (22.58%)             │
│ stage execs : 28/29 (96.55%)        │  new edges on : 11 (35.48%)            │
│ total execs : 107k                  │ total crashes : 12 (1 unique)          │
│  exec speed : 4704/sec              │  total tmouts : 0 (0 unique)           │
├─ fuzzing strategy yields ───────────┴───────────────┬─ path geometry ────────┤
│   bit flips : 1/920, 3/894, 4/813                   │    levels : 4          │
│  byte flips : 0/111, 0/86, 0/38                     │   pending : 6          │
│ arithmetics : 6/6184, 0/1022, 0/89                  │  pend fav : 0          │
│  known ints : 0/654, 0/2334, 0/1632                 │ own finds : 30         │
│  dictionary : 0/0, 0/0, 0/0                         │  imported : n/a        │
│       havoc : 16/77.0k, 1/15.0k                     │ stability : 100.00%    │
│        trim : 80.76%/65, 0.00%                      ├────────────────────────┘
└─────────────────────────────────────────────────────┘          [cpu000:  9%]
```
##  3. <a name='Cmodulefuzzing'></a>C module fuzzing

If C module dependencies are build with "afl-gcc", then coverage guidance from the C modules will be reported. This allows
bugs in the C modules to found (which are more likely to be exploitable). With Luarocks, this is achieved by appending

```
CC=afl-gcc
```
when installing a rock e.g.

```
luarocks install foo CC=afl-gcc
```

An [example](./examples/lua-cmsgpack/fuzz.sh) of reproducing [CVE-2018-11218](http://antirez.com/news/119) impacting [version 0.4.0-0 of lua-cmsgpack](https://github.com/antirez/lua-cmsgpack/tree/0.4.0) is given
which demonstrates how the fuzzing coverage extends across pure-Lua into C Lua modules.

##  4. <a name='OtherApproaches'></a>Other Approaches

It's possible to make a fuzzer along the lines of [afl-python](https://github.com/jwilk/python-afl) where debug instrumentation is used to provide coverage guidance. See [this gist](https://gist.github.com/stevenjohnstone/2236f632bb58697311cd01ea1cafbbc6) for a Lua implementation. 


##  5. <a name='AdvancedUsage'></a>Advanced Usage

###  5.1. <a name='Annotations'></a>Annotations

Fuzzing driven by code-coverage alone has limitations. Often, programs are structured so that a large proportion of code
can be covered while reaching very few states. There comes a point where the fuzzer gets no useful feedback from coverage
to guide future guesses and the fuzzer becomes no better than a dumb-fuzzer providing random inputs. It's possible to fuzz
the tradionally unfuzzable by adding "human-in-the-loop" annotations to code, following the example of
[Ijon](https://github.com/RUB-SysSec/ijon). Simple maze and obstacle course games are presented as examples of code where
code coverage can be very quickly saturated without reaching any interesting states at all. By adding "annotations" to the
code, the fuzzer can be guided to reach new and interesting states and be made to actually solve puzzles. This is quite
remarkable as the maze, for example, is difficult enough to defeat symbolic exectution engines due to the explosion of states.

####  5.1.1. <a name='SolvingaMaze'></a>Solving a Maze

In [./examples/maze](/examples/maze), a simple maze game is annotated with [afl_state](examples/maze/maze.lua#L31) so that
new player positions are reported back to the fuzzer, in addition to code coverage. This annotation allows the fuzzer to
find inputs which solve the puzzle in a matter of minutes. Without this annotation, the fuzzer can be left for hours without
success and it's unlikely that a good input would be found.

To try it out:

```
make # build afl-lua
cd examples/maze
./fuzz.sh
```

Here's a solution the fuzzer came up with after a few minutes:

![solution](./examples/maze/maze.svg)

####  5.1.2. <a name='ObstacleCourse'></a>Obstacle Course

[Ijon](https://github.com/RUB-SysSec/ijon) modifies afl-fuzz to implement primitives IJON_MAX and IJON_MIN which can be used to make the fuzzer choose inputs which maximize or minimize a given state value, respectively.
In afl-lua, equivalents are implemented without requiring changes to afl-fuzz. Instead, a table of previous max/min state values is stored by the annotation library and new values are communicated to afl-fuzz when those
change.

To demonstrate this functionality, an obstacle course game is driven by afl-lua with
a call to [afl_max](/examples/obstacles/obstacle.lua#L33) which prompts the fuzzer to choose inputs which maximise the
distance to the right which the player travels.

To try it out:

```
make
cd examples/obstacle
./fuzz.sh
```

![solution](./examples/obstacles/obstacle.svg)


###  5.2. <a name='PersistentMode'></a>Peristent Mode

To improve peformance of fuzzing, _persistent mode_ can be used. In this mode, the fuzzing process loops
a configurable number of times without exiting while trying new inputs. This reduces the overhead of forking
a new child process for every new input. The perfomance improvement can be around 3x but your milleage may vary.

The recipe is

```lua
for _ in afl.loop(1000) do
    local data = io.read("*all")
    local ok = pcall(target(data))
    if not ok then
        // flag an interesting error to AFL
        os.exit(1)
    end
end
```

This may not be suitable for all targets. For example, if the target leaks memory, file descriptor etc, then this will result in
resource exhaustion issues which are hard to debug. If the body of the loop keeps state between iterations, then
the stability metric may fall and fuzzing will become ineffective.




