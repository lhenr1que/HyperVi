-- Pure geometry for quick tiling; no Hyprland calls so it runs under plain lua.
local M = {}

-- area = {x, y, w, h}; gap_out/gap_in in px. Returns the rect for `where`, or nil.
function M.tile_rect(area, where, gap_in)
  local gi = (gap_in or 0) / 2
  local x, y, w, h = area.x, area.y, area.w, area.h
  local hw, hh = w / 2, h / 2
  local left  = { x = x,       w = hw - gi }
  local right = { x = x + hw + gi, w = hw - gi }
  local top   = { y = y,       h = hh - gi }
  local bot   = { y = y + hh + gi, h = hh - gi }
  local full_x, full_y = { x = x, w = w }, { y = y, h = h }
  local cols = { l = left, r = right, f = full_x }
  local rows = { t = top, b = bot, f = full_y }
  local map = {
    left = "lf", right = "rf", top = "ft", bottom = "fb",
    topleft = "lt", topright = "rt", bottomleft = "lb", bottomright = "rb", max = "ff",
  }
  local k = map[where]
  if not k then return nil end
  local c, r = cols[k:sub(1, 1)], rows[k:sub(2, 2)]
  return { x = c.x, y = r.y, w = c.w, h = r.h }
end

-- Keep one axis of `cur`, stretch the other over `area`. axis "h" = horizontal.
function M.axis_rect(area, cur, axis)
  if axis == "h" then return { x = area.x, y = cur.y, w = area.w, h = cur.h } end
  if axis == "v" then return { x = cur.x, y = area.y, w = cur.w, h = area.h } end
end

-- Push `cur` against one edge of `area` without resizing. dir: left right up down.
function M.pack_rect(area, cur, dir)
  local r = { x = cur.x, y = cur.y, w = cur.w, h = cur.h }
  if dir == "left" then r.x = area.x
  elseif dir == "right" then r.x = area.x + area.w - cur.w
  elseif dir == "up" then r.y = area.y
  elseif dir == "down" then r.y = area.y + area.h - cur.h
  else return nil end
  return r
end

function M.same(a, b, tol)
  tol = tol or 2
  return math.abs(a.x - b.x) <= tol and math.abs(a.y - b.y) <= tol
     and math.abs(a.w - b.w) <= tol and math.abs(a.h - b.h) <= tol
end

return M
