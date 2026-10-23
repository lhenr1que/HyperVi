package.path = (arg[0]:match("(.*/)") or "./") .. "?.lua;" .. package.path
local L = require("layouts")
local area = { x = 10, y = 20, w = 1000, h = 600 }
local function same(a, b, msg) for _, k in ipairs({ "x", "y", "w", "h" }) do assert(math.abs(a[k] - b[k]) < 1e-6, msg .. " " .. k .. " " .. a[k] .. " vs " .. b[k]) end end
local r = L.zone_rects(L.find("halves").zones, area, 0)
same(r[1], { x = 10, y = 20, w = 500, h = 600 }, "halves 1"); same(r[2], { x = 510, y = 20, w = 500, h = 600 }, "halves 2")
r = L.zone_rects(L.find("halves").zones, area, 10) -- 5 px off each side of the shared edge only
same(r[1], { x = 10, y = 20, w = 495, h = 600 }, "gap left"); same(r[2], { x = 515, y = 20, w = 495, h = 600 }, "gap right")
r = L.zone_rects(L.find("grid").zones, area, 10)
same(r[1], { x = 10, y = 20, w = 495, h = 295 }, "grid tl"); same(r[4], { x = 515, y = 325, w = 495, h = 295 }, "grid br")
assert(#L.zone_rects(L.find("thirds").zones, area, 0) == 3 and #L.zone_rects(L.find("main-side").zones, area, 0) == 3, "zone counts")
-- every preset covers the whole area when there is no gap
for _, p in ipairs(L.presets) do
  local total = 0
  for _, z in ipairs(L.zone_rects(p.zones, area, 0)) do total = total + z.w * z.h end
  assert(math.abs(total - area.w * area.h) < 1e-3, p.name .. " leaves holes or overlaps")
end
assert(L.find("nope") == nil, "unknown preset")
-- user layouts from the editor's JSON
L.load_user('[{"name":"user-a","label":"My one","gap":4,"zones":[[0,0,0.7,1],[0.7,0,0.3,0.5],[0.3,0.3,0.4,0.4]]},{"name":"bad","zones":[[0,0,2,1]]},{"name":"x y","zones":[[0,0,1,1]]},{"name":"user-b","zones":[]}]')
assert(#L.user == 1 and L.find("user-a").label == "My one" and L.find("user-a").gap == 4 and #L.find("user-a").zones == 3, "user layout parsed, bad ones dropped")
assert(L.find("grid") and L.find("bad") == nil, "presets stay, bad dropped")
L.load_user("not json {{"); assert(#L.user == 0, "garbage gives no layouts")
L.load_user('[{"name":"os","zones":[[0,0,1,1]],"x":os.exit()}]'); assert(#L.user == 0 or L.find("os"), "no env access")
L.load_user(nil); assert(#L.user == 0)
print("layouts ok")
