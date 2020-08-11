local cmsg = require("cmsgpack")
local cmsgsafe = require("cmsgpack.safe")

for _ in afl.loop(1000) do
    local data = io.read("*all")
    local u, err = cmsgsafe.unpack(data)
    if not err and u then
        local res = {cmsg.unpack(data)}
        cmsg.pack(table.unpack(res))
    end
end
