local maze = require("maze")
local annotations = require("annotations")
local grid = require("grid")

local m = maze:new(grid)
m.state_set = function(row, col)
    annotations.afl_map_set(row << 8 | col)
end

local i = 512
while i > 0 do
    local direction = io.read(1)
    local ok, v = pcall(m.move, m, direction)
    if not ok then
        print(v)
        break
    end

    if v then
        print("winner!")
        os.exit(1)
    end
    i = i - 1
end