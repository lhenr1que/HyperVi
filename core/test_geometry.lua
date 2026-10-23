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
local sz = { w = 200, h = 100 }
local x, y = g.place(area, sz, "center"); assert(x == 410 and y == 270, "center")
x, y = g.place(area, sz, "cursor", { pos = { x = 0, y = 0 } }); assert(x == 10 and y == 20, "cursor clamps to area")
x, y = g.place(area, sz, "cursor", { pos = { x = 500, y = 300 } }); assert(x == 400 and y == 250, "cursor")
x, y = g.place(area, sz, "cascade", { n = 2, step = 30 }); assert(x == 70 and y == 80, "cascade")
x, y = g.place(area, sz, "smart", { others = {} }); assert(x == 10 and y == 20, "smart empty goes top-left")
-- a window covering the left half pushes the new one to the right half, with zero overlap
x, y = g.place(area, sz, "smart", { others = { { x = 10, y = 20, w = 500, h = 600 } } })
assert(x >= 510, "smart avoids overlap, got " .. x)
local mons = { { x = 0, y = 0, w = 100, h = 50 }, { x = 100, y = 0, w = 100, h = 50 } }
assert(g.corner_at(mons, 0, 0, 2) == "topleft" and g.corner_at(mons, 99, 49, 2) == "bottomright", "corners monitor 1")
assert(g.corner_at(mons, 100, 0, 2) == "topleft" and g.corner_at(mons, 199, 0, 2) == "topright", "corners monitor 2")
assert(g.corner_at(mons, 50, 25, 2) == nil and g.corner_at(mons, 50, 0, 2) == nil and g.corner_at(mons, 500, 5, 2) == nil, "not a corner")
local st = {}
assert(not g.dwell(st, "topleft", 3) and not g.dwell(st, "topleft", 3), "needs 3 ticks")
assert(g.dwell(st, "topleft", 3), "fires on 3rd")
assert(not g.dwell(st, "topleft", 3), "fires once")
assert(not g.dwell(st, nil, 3) and not g.dwell(st, "topleft", 3) and not g.dwell(st, "topleft", 3) and g.dwell(st, "topleft", 3), "re-arms after leaving")
assert(not g.dwell({}, "topleft", 3) and not g.dwell(st, "topright", 3), "other corner restarts the count")
print("geometry ok")
