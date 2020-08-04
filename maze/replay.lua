local maze = require("maze")

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

local function sleep(n)
    os.execute("sleep " .. n)
end

local moves = io.read("*all")
for c in moves:gmatch(".") do
    m:draw()
    if m:move(c) then
        m:draw()
        break
    end
    sleep(0.5)
end
