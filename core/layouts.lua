-- Tiling presets: a layout is a list of zones, each {x, y, w, h} as fractions of the work area. Pure; runs under plain lua.
local M = {}

M.presets = {
  { name = "halves",      label = "Two columns",      zones = { { 0, 0, .5, 1 }, { .5, 0, .5, 1 } } },
  { name = "thirds",      label = "Three columns",    zones = { { 0, 0, 1 / 3, 1 }, { 1 / 3, 0, 1 / 3, 1 }, { 2 / 3, 0, 1 / 3, 1 } } },
  { name = "grid",        label = "Grid 2x2",         zones = { { 0, 0, .5, .5 }, { .5, 0, .5, .5 }, { 0, .5, .5, .5 }, { .5, .5, .5, .5 } } },
  { name = "main-side",   label = "Main and side",    zones = { { 0, 0, .6, 1 }, { .6, 0, .4, .5 }, { .6, .5, .4, .5 } } },
  { name = "wide-center", label = "Wide centre",      zones = { { 0, 0, .25, 1 }, { .25, 0, .5, 1 }, { .75, 0, .25, 1 } } },
}

-- Layouts made in the zone editor (kwinish-ui zones), read from layouts.json by M.load_user.
M.user = {}

function M.find(name)
  for _, list in ipairs({ M.user, M.presets }) do
    for _, p in ipairs(list) do if p.name == name then return p end end
  end
end

-- `text` is the editor's JSON: [{"name":..,"label":..,"gap":N?,"zones":[[x,y,w,h],..]},..]. The editor writes only
-- [A-Za-z0-9 -] inside strings, so the file is turned into a Lua table and evaluated with an empty environment.
-- Entries that are malformed (bad zones, odd names) are dropped; a bad file leaves no user layouts.
function M.load_user(text)
  M.user = {}
  if not text or text == "" then return end
  local src = text:gsub('"(%w+)"%s*:', "%1="):gsub("%[", "{"):gsub("%]", "}")
  local chunk = load("return " .. src, "layouts.json", "t", {})
  local ok, list = pcall(chunk or error)
  if not ok or type(list) ~= "table" then return end
  for _, l in ipairs(list) do
    local good = type(l) == "table" and type(l.name) == "string" and l.name:match("^[%w%-]+$") and type(l.zones) == "table" and #l.zones > 0
    for _, z in ipairs(good and l.zones or {}) do
      good = good and #z == 4 and z[1] >= 0 and z[2] >= 0 and z[3] > 0 and z[4] > 0 and z[1] + z[3] <= 1 + 1e-6 and z[2] + z[4] <= 1 + 1e-6
    end
    if good then
      M.user[#M.user + 1] = { name = l.name, label = type(l.label) == "string" and l.label or l.name, zones = l.zones, gap = tonumber(l.gap), user = true }
    end
  end
end

-- Pixel rects for `zones` inside `area` {x, y, w, h}. `gap` (the padding between windows) is taken half from each side
-- of an edge two zones share; edges on the border of the area stay where they are.
function M.zone_rects(zones, area, gap)
  local eps, half, out = 1e-6, (gap or 0) / 2, {}
  for _, z in ipairs(zones) do
    local x0, y0 = z[1], z[2]
    local x1, y1 = z[1] + z[3], z[2] + z[4]
    local l = x0 > eps and half or 0
    local t = y0 > eps and half or 0
    local r = x1 < 1 - eps and half or 0
    local b = y1 < 1 - eps and half or 0
    out[#out + 1] = {
      x = area.x + x0 * area.w + l, y = area.y + y0 * area.h + t,
      w = (x1 - x0) * area.w - l - r, h = (y1 - y0) * area.h - t - b,
    }
  end
  return out
end

return M
