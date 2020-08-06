local grid = require("grid")
local obstacle = require("obstacle")

local obs = obstacle:new(grid)
obs:draw()

local function sleep(n)
    os.execute("sleep " .. n)
end

local i = 1
local moves = io.read("*all")
for c in moves:gmatch(".") do
    local done = obs:move(c)
    obs:draw()
    print("move " .. i .. " " .. c)
    i = i + 1
    sleep(0.1)
    if done then
        break
    end
end
