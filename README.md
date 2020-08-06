# afl-lua
Fork of Lua adding AFL (https://github.com/google/afl) instrumentation to allow Lua scripts (not the VM itself) to be fuzzed.

## Building

On Linux (maybe other POSIX-y systems?), 

```
make
```

In the directory, there'll be an executable called "afl-lua".

## Try it out

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
## C module fuzzing

If C module dependencies are build with "afl-fuzz", then coverage guidance from the C modules will be reported. This allows
bugs in the C modules to found (which are more likely to be exploitable). With Luarocks, this is achieved by appending

```
CC=afl-fuzz
```
when installing a rock e.g.

```
luarocks install foo CC=afl-gcc
```

## Other Approaches

It's possible to make a fuzzer along the lines of [afl-python](https://github.com/jwilk/python-afl) where debug instrumentation is used to provide coverage guidance. See [this gist](https://gist.github.com/stevenjohnstone/2236f632bb58697311cd01ea1cafbbc6) for a Lua implementation. 


## Advanced Usage

### Annotations

#### Solving a Maze

It's possible to fuzz the tradionally unfuzzable by adding "human-in-the-loop" annotations to code, following the example of [Ijon](https://github.com/RUB-SysSec/ijon). In ./maze, a simple (but hard for fuzzers to solve) maze game
is annotated with AFL feedback which reveals the _state_ of the game which isn't reflected in simply
recording coverage. A [C Lua module](/annotations/annotations.c) is added which will record the current row and column in the game so that new pathways are revealed to AFL. Without this, the fuzzer would likely fail to find a solution as code coverage alone tells us little about where the player currently is in the maze.

To try it out:

```
make # build afl-lua
cd examples/maze
./fuzz.sh
```

Here's a solution the fuzzer came up with after a few minutes:

![solution](./examples/maze/maze.svg)

#### Obstacle Course

[Ijon](https://github.com/RUB-SysSec/ijon) modifies afl-fuzz to implement primitives IJON_MAX and IJON_MIN which can be used to make the fuzzer choose inputs which maximize or minimize a given state value, respectively.
In afl-lua, equivalents are implemented without requiring changes to afl-fuzz. Instead, a table of previous max/min state values is stored by the annotation library and new values are communicated to afl-fuzz when those
change.

To demonstrate this functionality, an obstacle course game is driven by afl-lua with ```afl_max``` which drives the fuzzer to keep seeking states to the right of the current state.

To try it out:

```
make
cd examples/obstacle
./fuzz.sh
```

![solution](./examples/obstacles/obstacle.svg)




