local maze = require("maze")
local grid = require("grid")

local m = maze:new(grid())
m:draw()

local function sleep(n)
    os.execute("sleep " .. n)
end

local i = 1
local moves = io.read("*all")
for c in moves:gmatch(".") do
    local done = m:move(c)
    m:draw()
    print("move " .. i .. " " .. c)
    i = i + 1
    sleep(0.01)
    if done then
        break
    end
end
