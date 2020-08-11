local grid = require("grid")
local obstacle = require("obstacle")


for _ in afl.loop(10000) do
    local obs = obstacle:new(grid())
    obs.afl_max = afl.max
    local data = io.read("*all")
    for direction in data:gmatch(".") do
        local ok, v = pcall(obs.move, obs, direction)
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
