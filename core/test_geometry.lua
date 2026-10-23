package.path = (arg[0]:match("(.*/)") or "./") .. "?.lua;" .. package.path
local g = require("geometry")
local area = { x = 10, y = 20, w = 1000, h = 600 }
local function eq(a, b, msg) assert(g.same(a, b, 0), msg) end
eq(g.tile_rect(area, "left", 0),        { x = 10,  y = 20,  w = 500, h = 600 }, "left")
eq(g.tile_rect(area, "right", 0),       { x = 510, y = 20,  w = 500, h = 600 }, "right")
eq(g.tile_rect(area, "topright", 0),    { x = 510, y = 20,  w = 500, h = 300 }, "topright")
eq(g.tile_rect(area, "bottomleft", 0),  { x = 10,  y = 320, w = 500, h = 300 }, "bottomleft")
eq(g.tile_rect(area, "max", 0),         area, "max")
eq(g.tile_rect(area, "left", 10),       { x = 10,  y = 20,  w = 495, h = 600 }, "left with gap")
eq(g.tile_rect(area, "right", 10),      { x = 515, y = 20,  w = 495, h = 600 }, "right with gap")
assert(g.tile_rect(area, "nope") == nil, "unknown")
eq(g.axis_rect(area, { x = 100, y = 200, w = 300, h = 150 }, "h"), { x = 10, y = 200, w = 1000, h = 150 }, "axis h")
eq(g.axis_rect(area, { x = 100, y = 200, w = 300, h = 150 }, "v"), { x = 100, y = 20, w = 300, h = 600 }, "axis v")
local c = { x = 100, y = 200, w = 300, h = 150 }
eq(g.pack_rect(area, c, "left"),  { x = 10,  y = 200, w = 300, h = 150 }, "pack left")
eq(g.pack_rect(area, c, "right"), { x = 710, y = 200, w = 300, h = 150 }, "pack right")
eq(g.pack_rect(area, c, "up"),    { x = 100, y = 20,  w = 300, h = 150 }, "pack up")
eq(g.pack_rect(area, c, "down"),  { x = 100, y = 470, w = 300, h = 150 }, "pack down")
assert(g.pack_rect(area, c, "x") == nil, "pack unknown")
print("geometry ok")
