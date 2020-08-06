local grid = require("grid")
local annotations = require("annotations")
local obstacle = require("obstacle")

local obs = obstacle:new(grid)
obs.afl_max = annotations.afl_max

local i = 1024
while i > 0 do
    local direction = io.read(1)
    local ok, v = pcall(obs.move, obs, direction)
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
