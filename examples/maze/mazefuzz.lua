local maze = require("maze")
local grid = require("grid")

local max_iterations = 150

for _ in afl.loop(1000) do

    local directions = io.read("*all")
    local m = maze:new(grid())
    m.state_set = function(row, col)
        afl.map_set(row << 8 | col)
    end
    local i = 1
    for direction in directions:gmatch(".") do
        if i > max_iterations then
            break
        end
        i = i + 1
        local ok, v = pcall(m.move, m, direction)
        if not ok then
            print(v)
            break
        end

        if v then
            print("winner!")
            os.exit(1)
        end
    end
end
