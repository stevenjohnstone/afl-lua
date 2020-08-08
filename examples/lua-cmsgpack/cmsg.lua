local cmsg = require("cmsgpack")
local cmsgsafe = require("cmsgpack.safe")
--local inspect = require("inspect")

local function fuzz()
    local data = io.read("*all")
    local u, err = cmsgsafe.unpack(data)
    --print("err", inspect(err))
    if not err and u then
        local res = {cmsg.unpack(data)}
        --print(inspect(res))
        cmsg.pack(table.unpack(res))
    end
end
fuzz()
