package.path = (arg[0]:match("(.*/)") or "./") .. "?.lua;" .. package.path
local M = require("modes")
assert(M.engine("tiling") == "dwindle" and M.engine("scrolling") == "scrolling" and M.engine("floating") == "dwindle")
assert(M.from_engine("scrolling") == "scrolling" and M.from_engine("dwindle") == "tiling" and M.from_engine("master") == "tiling" and M.from_engine(nil) == "tiling")
assert(M.next("tiling") == "scrolling" and M.next("scrolling") == "floating" and M.next("floating") == "tiling" and M.next(nil) == "scrolling")
assert(M.next("tiling", true) == "floating" and M.next("floating", true) == "scrolling" and M.next("scrolling", true) == "tiling")
local t = M.parse('{"10":"floating","2":"scrolling","3":"weird","0":"floating","x":"floating"}')
assert(t["10"] == "floating" and t["2"] == "scrolling" and t["3"] == nil and t["0"] == nil and t.x == nil, "bad entries dropped")
assert(M.serialize(t) == '{"2":"scrolling","10":"floating"}', M.serialize(t))
assert(M.parse(M.serialize(t))["10"] == "floating" and next((M.parse(nil))) == nil and next((M.parse("garbage"))) == nil)
local r = M.open_rect({ x = 100, y = 50, w = 2000, h = 1000 }, 2) -- 1100x680 centred, step 2 = 0 offset
assert(r.w == 1100 and r.h == 680 and r.x == 100 + 450 and r.y == 50 + 160, "centred at step 2")
local a, b = M.open_rect({ x = 0, y = 0, w = 1000, h = 800 }, 0), M.open_rect({ x = 0, y = 0, w = 1000, h = 800 }, 1)
assert(b.x - a.x == 24 and b.y - a.y == 24 and a.w == 640 and a.h == 544, "cascade steps and small-screen size")
assert(M.open_rect({ x = 0, y = 0, w = 1000, h = 800 }, 5).x == a.x, "cascade wraps every 5")
local c = M.parse_columns('{"2":3,"4":2,"5":9,"0":1,"x":1}')
assert(c["2"] == 3 and c["4"] == 2 and c["5"] == nil and c["0"] == nil and c.x == nil, "columns parsed, bad dropped")
assert(M.serialize_columns(c) == '{"2":3,"4":2}' and M.parse_columns(M.serialize_columns(c))["2"] == 3 and next((M.parse_columns(nil))) == nil)
print("modes ok")

for _, n in ipairs({ "2.5", "2.0", "2e0", "2junk", "-1", "null" }) do
  assert(next((M.parse_columns('{"1":' .. n .. '}'))) == nil, "reject non-integer token: " .. n)
end
