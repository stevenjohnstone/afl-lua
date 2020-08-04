local maze = require("maze")
local annotations = require("annotations")


local asciigrid = {
    "+-+---+---+",
    "| |     |#|",
    "| | --+ | |",
    "| |   | | |",
    "| +-- | | |",
    "|     |   |",
    "+-----+---+"
}

local grid = {}
for _, row in ipairs(asciigrid) do
    local r = {}
    for c in row:gmatch(".") do
        table.insert(r, c)
    end
    table.insert(grid, r)
end

local m = maze:new(grid)
m.afl_set = annotations.afl_set

while true do
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
end

