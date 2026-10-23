-- kwinish core for Hyprland 0.55+ (Lua config). Load with:
--   package.path = "/path/to/kwinish/?.lua;" .. package.path
--   local kw = require("kwinish")
--   hl.bind("SUPER + Left", function() kw.tile("left") end)
local geometry = require("geometry")
local M = {}

-- shortcut: restore rects live in this table, so a config reload forgets them; move to window tags to survive reloads.
local saved = {}
local hot_started = false

local function xy(v, a, b) return v[a] or v[1], v[b] or v[2] end

-- Height of the title bar drawn above a floating window: the kwinish-bars plugin's own setting while it is loaded,
-- else M.bar_height (set it to 28 when the fallback layer-shell bars are in use), else 0.
local function bar_height()
  local ok, plugins = pcall(hl.get_loaded_plugins)
  if ok and type(plugins) == "table" then
    for _, p in ipairs(plugins) do
      if p.name == "kwinish-bars" then
        local on = select(2, pcall(hl.get_config, "plugin:kwinish:enabled"))
        if on == false then return 0 end
        local ok2, h = pcall(hl.get_config, "plugin:kwinish:bar_height")
        if ok2 and type(h) == "number" then return h end
      end
    end
  end
  return M.bar_height or 0
end

-- The plugin keeps the pre-snap size so that dragging the bar of a snapped window gives the old size back.
local function plugin_call(name, ...)
  local p = hl.plugin and hl.plugin.kwinish
  if p and p[name] then pcall(p[name], ...) end
end

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
  local bh = bar_height() -- the bar sits above the window, so the window starts below it
  return { x = mon.x + l, y = mon.y + t + bh, w = mw - l - rr, h = mh - t - b - bh }
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
    plugin_call("forget", w.address)
    put(w, s.rect)
    if not s.was_floating then hl.dispatch(hl.dsp.window.float({ action = "disable", window = "address:" .. w.address })) end
    return
  end
  if not s then saved[w.address] = { rect = rect_of(w), was_floating = w.floating, key = key }
  else
    -- moved or resized since the last snap (e.g. unsnapped by dragging): that geometry is the one to come back to
    if s.snap and not geometry.same(rect_of(w), s.snap) then s.rect, s.was_floating = rect_of(w), w.floating end
    s.key = key
  end
  if not w.floating then hl.dispatch(hl.dsp.window.float({ action = "enable", window = "address:" .. w.address })) end
  put(w, r)
  saved[w.address].snap = r
  local o = saved[w.address].rect
  plugin_call("remember", w.address, o.x, o.y, o.w, o.h, r.x, r.y, r.w, r.h)
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

-- dir: left right up down. Floats the window, like tile().
function M.pack(dir, opts)
  local w = target(opts); if not w then return end
  local r = geometry.pack_rect(work_area(w.monitor), rect_of(w), dir)
  if r then place(w, r, "pack:" .. dir) end
end

-- Show desktop: stash every window of the active workspace in a special workspace, or bring them back.
-- opts.filter(w) limits which windows are stashed (tests use it; default is all).
-- shortcut: origins live in a Lua table, so a config reload forgets them; move to window tags to survive reloads.
local STASH = "special:kwinish_desktop"
local stashed = {}
function M.show_desktop(opts)
  local sel = function(w) return "address:" .. w.address end
  if next(stashed) then
    for addr, ws in pairs(stashed) do
      hl.dispatch(hl.dsp.window.move({ workspace = ws, follow = false, window = "address:" .. addr }))
    end
    stashed = {}
    return
  end
  local ws = hl.get_active_workspace()
  if not ws then return end
  local filter = opts and opts.filter
  for _, w in ipairs(hl.get_workspace_windows(ws.id)) do
    if w.mapped and not w.hidden and (not filter or filter(w)) then
      stashed[w.address] = ws.id
      hl.dispatch(hl.dsp.window.move({ workspace = STASH, follow = false, window = sel(w) }))
    end
  end
end

-- Keep above: windows in `above` are raised again whenever focus changes (floating windows only).
local above, below = {}, {}
local hooked = false
-- The state also goes into a window tag so other programs (the window menu) can read it from `hyprctl clients`.
local function set_tag(w, name, on)
  hl.dispatch(hl.dsp.window.tag({ tag = (on and "+" or "-") .. name, window = "address:" .. w.address }))
end

local function has_tag(w, name)
  for _, t in ipairs(w.tags or {}) do if t == name then return true end end
  return false
end

function M.keep_above(opts)
  local w = target(opts); if not w then return end
  above[w.address] = (not above[w.address]) or nil
  set_tag(w, "kwinish-above", above[w.address] ~= nil)
  if above[w.address] then
    below[w.address] = nil; set_tag(w, "kwinish-below", false)
    hl.dispatch(hl.dsp.window.alter_zorder({ mode = "top", window = "address:" .. w.address }))
  end
end

-- Keep below: sent to the bottom of the floating stack, and again whenever focus moves.
function M.keep_below(opts)
  local w = target(opts); if not w then return end
  below[w.address] = (not below[w.address]) or nil
  set_tag(w, "kwinish-below", below[w.address] ~= nil)
  if below[w.address] then
    above[w.address] = nil; set_tag(w, "kwinish-above", false)
    hl.dispatch(hl.dsp.window.alter_zorder({ mode = "bottom", window = "address:" .. w.address }))
  end
end

-- No title bar, border or rounded corners (and hidden from screen capture): toggles, state kept in tags.
local function toggle_noframe(w)
  local on = not has_tag(w, "kwinish-noframe")
  set_tag(w, "kwinish-noframe", on)
  local sel = "address:" .. w.address
  for _, prop in ipairs({ "border_size", "rounding" }) do
    hl.dispatch(hl.dsp.window.set_prop({ prop = prop, value = on and "0" or "unset", window = sel }))
  end
  plugin_call("set_bar_hidden", w.address, on)
end

local function toggle_nocapture(w)
  local on = not has_tag(w, "kwinish-nocapture")
  set_tag(w, "kwinish-nocapture", on)
  hl.dispatch(hl.dsp.window.set_prop({ prop = "no_screen_share", value = on and "1" or "unset", window = "address:" .. w.address }))
end
-- opts.filter(w) limits both features to matching windows (tests use it).
-- opts: placement = "smart"|"cascade"|"center"|"cursor" (new floating windows), autoraise_ms = delay before a focused floating window is raised.
function M.setup(opts)
  if hooked then return end
  hooked = true
  opts = opts or {}
  local function raise(addr)
    hl.dispatch(hl.dsp.window.alter_zorder({ mode = "top", window = "address:" .. addr }))
  end
  -- the event fires before the newly focused window is raised, so raise on a short timer
  hl.on("window.active", function()
    local cur = hl.get_active_window()
    local cur_addr = cur and cur.address
    hl.timer(function()
      for addr in pairs(above) do
        if hl.get_window("address:" .. addr) then raise(addr) else above[addr] = nil end
      end
      for addr in pairs(below) do
        if hl.get_window("address:" .. addr) then
          hl.dispatch(hl.dsp.window.alter_zorder({ mode = "bottom", window = "address:" .. addr }))
        else below[addr] = nil end
      end
    end, { timeout = 30, type = "oneshot" })
    if opts.autoraise_ms and cur_addr then
      hl.timer(function()
        local now = hl.get_active_window()
        if now and now.address == cur_addr and now.floating and (not opts.filter or opts.filter(now)) then raise(cur_addr) end
      end, { timeout = opts.autoraise_ms, type = "oneshot" })
    end
  end)
  if opts.placement then
    hl.on("window.open", function(w)
      local addr = w.address -- re-fetched below: the window may be gone when the timer fires
      hl.timer(function()
        local win = hl.get_window("address:" .. addr)
        if not win or not win.mapped or not win.floating or not win.monitor or not win.workspace then return end
        if opts.filter and not opts.filter(win) then return end
        if win.xwayland and win.class == "" and win.title == "" then return end -- popups position themselves
        local size, others = rect_of(win), {}
        for _, o in ipairs(hl.get_workspace_windows(win.workspace.id)) do
          if o.address ~= addr and o.mapped and not o.hidden then others[#others + 1] = rect_of(o) end
        end
        local pos = hl.get_cursor_pos()
        local x, y = geometry.place(work_area(win.monitor), size, opts.placement,
          { others = others, n = #others, pos = pos and { x = pos.x or pos[1], y = pos.y or pos[2] } })
        if x then hl.dispatch(hl.dsp.window.move({ x = math.floor(x + 0.5), y = math.floor(y + 0.5), window = "address:" .. addr })) end
      end, { timeout = 30, type = "oneshot" })
    end)
  end
end

-- Entry point for the window menu (kwinish-ui): action is "tile:left", "axis:h", "pack:up", "above", "float",
-- "fullscreen", "close" or "workspace:N", applied to the window with that address.
function M.window_action(addr, action)
  local w = hl.get_window("address:" .. addr)
  if not w then return end
  local kind, arg = action:match("^(%w+):?(.*)$")
  local sel = "address:" .. addr
  if kind == "tile" then M.tile(arg, { window = w })
  elseif kind == "axis" then M.maximize_axis(arg, { window = w })
  elseif kind == "pack" then M.pack(arg, { window = w })
  elseif kind == "above" then M.keep_above({ window = w })
  elseif kind == "float" then hl.dispatch(hl.dsp.window.float({ action = "toggle", window = sel }))
  elseif kind == "fullscreen" then hl.dispatch(hl.dsp.window.fullscreen({ window = sel }))
  elseif kind == "close" then hl.dispatch(hl.dsp.window.close({ window = sel }))
  elseif kind == "workspace" and arg == "new" then
    local top = 0
    for _, ws in ipairs(hl.get_workspaces()) do if ws.id and ws.id > top then top = ws.id end end
    hl.dispatch(hl.dsp.window.move({ workspace = tostring(top + 1), window = sel }))
  elseif kind == "workspace" then hl.dispatch(hl.dsp.window.move({ workspace = arg, window = sel }))
  elseif kind == "pin" then hl.dispatch(hl.dsp.window.pin({ action = "toggle", window = sel }))
  elseif kind == "maximize" then hl.dispatch(hl.dsp.window.fullscreen({ mode = "maximized", window = sel }))
  elseif kind == "minimize" then hl.dispatch(hl.dsp.window.move({ workspace = "special:minimized", follow = false, window = sel }))
  elseif kind == "below" then M.keep_below({ window = w })
  elseif kind == "noframe" then toggle_noframe(w)
  elseif kind == "nocapture" then toggle_nocapture(w)
  end
end

-- Open the window menu for the focused window at the pointer.
function M.menu(opts)
  local w = (opts and opts.window) or hl.get_active_window()
  if w then hl.exec_cmd("kwinish-ui menu " .. w.address) end
end

-- Hot corners. opts.corners maps topleft/topright/bottomleft/bottomright to "show_desktop", "menu",
-- { exec = "command" } or a function; opts.delay_ms (dwell, default 300), opts.edge px (default 2), opts.tick_ms (default 100).
-- Lua sees no pointer-motion events, so a repeating timer polls the cursor; it cannot be stopped except by a config reload.
function M.hotcorners(opts)
  if hot_started then return end
  hot_started = true
  local corners, edge = opts.corners or {}, opts.edge or 2
  local tick = opts.tick_ms or 100
  local need = math.max(1, math.ceil((opts.delay_ms or 300) / tick))
  local st = {}
  local function run(action)
    if type(action) == "function" then action()
    elseif action == "show_desktop" then M.show_desktop()
    elseif action == "menu" then M.menu()
    elseif type(action) == "table" and action.exec then hl.exec_cmd(action.exec) end
  end
  hl.timer(function()
    local pos = hl.get_cursor_pos()
    if not pos then return end
    local rects = {}
    for _, m in ipairs(hl.get_monitors()) do
      local sc = m.scale or 1
      local w, h = m.width / sc, m.height / sc
      if (m.transform or 0) % 2 == 1 then w, h = h, w end
      rects[#rects + 1] = { x = m.x, y = m.y, w = w, h = h }
    end
    local c = geometry.corner_at(rects, pos.x or pos[1], pos.y or pos[2], edge)
    if geometry.dwell(st, c and corners[c] and c or nil, need) then run(corners[c]) end
  end, { timeout = tick, type = "repeat" })
end

-- Tell the plugin where each snap zone (left, right, top) puts a window, title bar included, so its drag preview shows
-- exactly what releasing the window there will do.
function M.publish_zones()
  local p = hl.plugin and hl.plugin.kwinish
  if not (p and p.set_zone) then return end
  pcall(p.clear_zones)
  local gi = num(hl.get_config("general.gaps_in"), "left")
  local bh = bar_height()
  for _, mon in ipairs(hl.get_monitors()) do
    local area = work_area(mon)
    for zone, where in pairs({ left = "left", right = "right", top = "max" }) do
      local r = geometry.tile_rect(area, where, gi)
      pcall(p.set_zone, mon.name, zone, r.x, r.y - bh, r.w, r.h + bh)
    end
  end
end

-- Zones depend on monitors, gaps and the bar height: republish when those can change (once per Lua state).
if not KWINISH_ZONE_HOOKS then
  KWINISH_ZONE_HOOKS = true
  for _, ev in ipairs({ "monitor.layout_changed", "monitor.added", "config.reloaded", "window.active" }) do
    hl.on(ev, function() local kw = package.loaded.kwinish; if kw then kw.publish_zones() end end)
  end
end
M.publish_zones()

M._work_area, M._rect_of = work_area, rect_of -- exposed for spikes and tests
return M
