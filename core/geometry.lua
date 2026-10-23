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

local function clamp(v, lo, hi) if hi < lo then return lo end return math.max(lo, math.min(hi, v)) end

local function overlap(a, b)
  local w = math.min(a.x + a.w, b.x + b.w) - math.max(a.x, b.x)
  local h = math.min(a.y + a.h, b.y + b.h) - math.max(a.y, b.y)
  if w <= 0 or h <= 0 then return 0 end
  return w * h
end

-- Top-left for a window of `size` {w,h}. Policies: center, cursor (needs pos {x,y}), cascade (n = index), smart (others = rects).
function M.place(area, size, policy, o)
  o = o or {}
  local maxx, maxy = area.x + area.w - size.w, area.y + area.h - size.h
  if policy == "center" then
    return clamp(area.x + (area.w - size.w) / 2, area.x, maxx), clamp(area.y + (area.h - size.h) / 2, area.y, maxy)
  elseif policy == "cursor" then
    return clamp(o.pos.x - size.w / 2, area.x, maxx), clamp(o.pos.y - size.h / 2, area.y, maxy)
  elseif policy == "cascade" then
    local step = o.step or 32
    local nx = math.max(1, math.floor((maxx - area.x) / step) + 1)
    local ny = math.max(1, math.floor((maxy - area.y) / step) + 1)
    local n = (o.n or 0)
    return area.x + (n % nx) * step, area.y + (n % ny) * step
  elseif policy == "smart" then
    local step = math.max(20, math.floor(math.max(maxx - area.x, maxy - area.y) / 24))
    local best, bx, by = nil, area.x, area.y
    local y = area.y
    while true do
      local x = area.x
      while true do
        local r, cost = { x = x, y = y, w = size.w, h = size.h }, 0
        for _, other in ipairs(o.others or {}) do cost = cost + overlap(r, other) end
        if not best or cost < best then best, bx, by = cost, x, y end
        if x >= maxx then break end
        x = math.min(x + step, maxx)
      end
      if y >= maxy then break end
      y = math.min(y + step, maxy)
    end
    return bx, by
  end
end

function M.same(a, b, tol)
  tol = tol or 2
  return math.abs(a.x - b.x) <= tol and math.abs(a.y - b.y) <= tol
     and math.abs(a.w - b.w) <= tol and math.abs(a.h - b.h) <= tol
end

return M
