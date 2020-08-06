local maze = {}
maze.__index = maze

function maze:new(grid)
    grid[2][2] = "X"
    return setmetatable(
        {
            grid = grid,
            row = 2,
            col = 2,
            state_set = function(_, _)
            end,
        },
        self
    )
end

function maze:draw()
    io.write("\027[H\027[2J")
    for _, row in ipairs(self.grid) do
        for _, cell in ipairs(row) do
            io.write(cell)
        end
        print("")
    end
    print("")
end

function maze:move(input)
    local row, col = self.row, self.col
    self.state_set(row, col)
    if input == "w" then
        row = row - 1
    elseif input == "s" then
        row = row + 1
    elseif input == "a" then
        col = col - 1
    elseif input == "d" then
        col = col + 1
    else
        error("wrong command")
    end
    if row < 1 or col < 1 or row > #self.grid or col > #self.grid[1] then
        return false
    end

    local r = self.grid[self.row]
    r[self.col] = " " -- clear the marker
    local lastr = r
    r = self.grid[row]

    if r[col] == "#" then
        -- win
        r[col] = "X"
        return true
    end

    if r[col] == " " then
        self.row = row
        self.col = col
        r[self.col] = "X"
        return false
    else
        -- collision
        lastr[self.col] = "x"
        return false
    end
end

return maze
