local asciigrid = {
    "+-+-------------+",
    "| |             |",
    "| | +-----* *---+",
    "|   |           |",
    "+---+-* *-------+",
    "|               |",
    "+ +-------------+",
    "| |       |   |#|",
    "| | *---+ * * * |",
    "| |     |   |   |",
    "| +---* +-------+",
    "|               |",
    "+---------------+"
}

local grid = {}
for _, row in ipairs(asciigrid) do
    local r = {}
    for c in row:gmatch(".") do
        table.insert(r, c)
    end
    table.insert(grid, r)
end
return grid