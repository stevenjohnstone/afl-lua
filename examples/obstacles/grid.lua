local asciiobstacle = {
    "+----------+------------------------------------------------------+--------------+",
    "|          |                                             *        |              #",
    "|          |                     ------+            *             |              #",
    "|          |                           |        *        *        |     |        #",
    "|          |        +-----+       |    |     *      *             |     |        #",
    "|          |__      |     |       |    |  *     *        *        |     |        #",
    "|             |     |     |       -----+     *      *             |     |        #",
    "|          |  |     |     |                     *        *        |     |        #",
    "|          |        |     |                         *                   |        #",
    "|          |        |     |                              *        |     |        #",
    "+----------+--------+-----+---------------------------------------+-----+--------+"
}
return function()
    local obstacle = {}
    for _, row in ipairs(asciiobstacle) do
        local r = {}
        for c in row:gmatch(".") do
            table.insert(r, c)
        end
        table.insert(obstacle, r)
    end
    return obstacle
end
