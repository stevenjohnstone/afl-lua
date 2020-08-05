local maze = require("maze")
local annotations = require("annotations")
local grid = require("grid")

local m = maze:new(grid)
m.afl_set = annotations.afl_set

local i = 512
while i > 0 do
    m:draw()
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
