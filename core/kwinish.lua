-- kwinish core for Hyprland 0.55+ (Lua config). Load with:
--   package.path = "/path/to/kwinish/?.lua;" .. package.path
--   local kw = require("kwinish")
--   hl.bind("SUPER + Left", function() kw.tile("left") end)
local geometry = require("geometry")
local M = {}

-- shortcut: restore rects live in this table, so a config reload forgets them; move to window tags to survive reloads.
local saved = {}

local function xy(v, a, b) return v[a] or v[1], v[b] or v[2] end

local function num(v, side)
  if type(v) == "table" then return v[side] or 0 end
  return v or 0
end

-- Work area of the window's monitor in layout coordinates, minus bars and outer gaps.
local function work_area(mon)
  local scale = mon.scale or 1
  local mw, mh = mon.width / scale, mon.height / scale
  if (mon.transform or 0) % 2 == 1 then mw, mh = mh, mw end
  local r = mon.reserved or {}
  local go = hl.get_config("general.gaps_out")
  local l, t = num(r.left or r[1], "left") + num(go, "left"), num(r.top or r[2], "top") + num(go, "top")
  local rr, b = num(r.right or r[3], "right") + num(go, "right"), num(r.bottom or r[4], "bottom") + num(go, "bottom")
  return { x = mon.x + l, y = mon.y + t, w = mw - l - rr, h = mh - t - b }
end

local function rect_of(w)
  local x, y = xy(w.at, "x", "y")
  local ww, hh = xy(w.size, "x", "y")
  return { x = x, y = y, w = ww, h = hh }
end

local function put(w, r)
  local sel = "address:" .. w.address
  hl.dispatch(hl.dsp.window.resize({ x = math.floor(r.w + 0.5), y = math.floor(r.h + 0.5), exact = true, window = sel }))
  hl.dispatch(hl.dsp.window.move({ x = math.floor(r.x + 0.5), y = math.floor(r.y + 0.5), window = sel }))
end

local function target(opts)
  local w = opts and opts.window or hl.get_active_window()
  if not w or not w.mapped or w.fullscreen ~= 0 or not w.monitor then return nil end
  return w
end

-- Apply `r`; the same request again on an already-placed window restores the original rect.
local function place(w, r, key)
  local s = saved[w.address]
  if s and s.key == key and geometry.same(rect_of(w), r) then
    saved[w.address] = nil
    put(w, s.rect)
    if not s.was_floating then hl.dispatch(hl.dsp.window.float({ action = "disable", window = "address:" .. w.address })) end
    return
  end
  if not s then saved[w.address] = { rect = rect_of(w), was_floating = w.floating, key = key }
  else s.key = key end
  if not w.floating then hl.dispatch(hl.dsp.window.float({ action = "enable", window = "address:" .. w.address })) end
  put(w, r)
end

-- where: left right top bottom topleft topright bottomleft bottomright max
function M.tile(where, opts)
  local w = target(opts); if not w then return end
  local gi = num(hl.get_config("general.gaps_in"), "left")
  local r = geometry.tile_rect(work_area(w.monitor), where, gi)
  if r then place(w, r, "tile:" .. where) end
end

-- axis: "h" or "v". Floats the window (restores on repeat).
function M.maximize_axis(axis, opts)
  local w = target(opts); if not w then return end
  local r = geometry.axis_rect(work_area(w.monitor), rect_of(w), axis)
  if r then place(w, r, "axis:" .. axis) end
end

M._work_area, M._rect_of = work_area, rect_of -- exposed for spikes and tests
return M
