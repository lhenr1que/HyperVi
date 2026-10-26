package.path = (arg[0]:match("(.*/)") or "./") .. "?.lua;" .. package.path
local A = require("assist")
local rects = { { x = 0, y = 0, w = 500, h = 600 }, { x = 500, y = 0, w = 500, h = 600 }, { x = 0, y = 600, w = 1000, h = 400 } }
local function win(a, x, y, w, h) return { address = a, class = "c", title = "t" .. a, x = x, y = y, w = w, h = h } end
local p = A.plan(rects, { win("a", 0, 0, 500, 600) }, 1)
-- a sits on zone 1 (which was placed): not a candidate, so nothing to offer
assert(p == nil, "window already in a zone is no candidate")
p = A.plan(rects, { win("a", 2, 598, 1000, 400), win("b", 600, 100, 100, 100) }, 1)
-- a is on zone 3: it takes zone 3, b is the loose one, zone 2 is the first empty
assert(p and p.zone == 2 and #p.candidates == 1 and p.candidates[1].address == "b", "first empty zone, loose windows only")
assert(A.plan(rects, { win("b", 600, 100, 100, 100) }, 2).zone == 1, "lowest empty index")
assert(A.plan(rects, {}, 1) == nil, "no candidates")
assert(A.plan({ rects[1] }, { win("b", 600, 100, 1, 1) }, 1) == nil, "no empty zone")
local j = A.json('DP"1', { zone = 2, rect = { x = 500.4, y = 0, w = 500, h = 600 }, candidates = { { address = "0x1", class = "k", title = 'a"b\n' } } })
assert(j:find('"zone":2,"rect":[500,0,500,600]', 1, true) and j:find('a\\u0022b\\u000a', 1, true) and not j:find("\n"), j)
print("assist ok")
