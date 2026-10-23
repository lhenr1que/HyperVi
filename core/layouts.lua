-- Tiling presets: a layout is a list of zones, each {x, y, w, h} as fractions of the work area. Pure; runs under plain lua.
local M = {}

M.presets = {
  { name = "halves",      label = "Two columns",      zones = { { 0, 0, .5, 1 }, { .5, 0, .5, 1 } } },
  { name = "thirds",      label = "Three columns",    zones = { { 0, 0, 1 / 3, 1 }, { 1 / 3, 0, 1 / 3, 1 }, { 2 / 3, 0, 1 / 3, 1 } } },
  { name = "grid",        label = "Grid 2x2",         zones = { { 0, 0, .5, .5 }, { .5, 0, .5, .5 }, { 0, .5, .5, .5 }, { .5, .5, .5, .5 } } },
  { name = "main-side",   label = "Main and side",    zones = { { 0, 0, .6, 1 }, { .6, 0, .4, .5 }, { .6, .5, .4, .5 } } },
  { name = "wide-center", label = "Wide centre",      zones = { { 0, 0, .25, 1 }, { .25, 0, .5, 1 }, { .75, 0, .25, 1 } } },
}

function M.find(name)
  for _, p in ipairs(M.presets) do if p.name == name then return p end end
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
