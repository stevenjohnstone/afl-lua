local maze = require("maze")
local grid = require("grid")

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
    sleep(0.1)
end
