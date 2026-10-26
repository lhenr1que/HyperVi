-- kwinish core for Hyprland 0.55+ (Lua config). Load with:
--   package.path = "/path/to/kwinish/?.lua;" .. package.path
--   local kw = require("kwinish")
--   hl.bind("SUPER + Left", function() kw.tile("left") end)
local geometry = require("geometry")
local layouts = require("layouts")
local rules = require("rules")
local modes = require("modes")
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
-- Tiled windows are the compositor's: snapping, zones, pack and axis leave them alone.
local function place(w, r, key)
  if not w.floating then return end
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
  local on = not has_tag(w, "kwinish-above") -- the tag outlives a core reload, the table does not
  above[w.address] = on or nil
  set_tag(w, "kwinish-above", on)
  if on then
    below[w.address] = nil; set_tag(w, "kwinish-below", false)
    hl.dispatch(hl.dsp.window.alter_zorder({ mode = "top", window = "address:" .. w.address }))
  end
end

-- Keep below: sent to the bottom of the floating stack, and again whenever focus moves.
function M.keep_below(opts)
  local w = target(opts); if not w then return end
  local on = not has_tag(w, "kwinish-below")
  below[w.address] = on or nil
  set_tag(w, "kwinish-below", on)
  if on then
    above[w.address] = nil; set_tag(w, "kwinish-above", false)
    hl.dispatch(hl.dsp.window.alter_zorder({ mode = "bottom", window = "address:" .. w.address }))
  end
end

-- After a core reload the lists are empty while the windows keep their tags: read them back.
pcall(function()
  for _, w in ipairs(hl.get_windows()) do
    if has_tag(w, "kwinish-above") then above[w.address] = true end
    if has_tag(w, "kwinish-below") then below[w.address] = true end
  end
end)

-- Window addresses are reused: forget what was kept for a window that closed.
function M.forget_window(addr)
  saved[addr], above[addr], below[addr], stashed[addr] = nil, nil, nil, nil
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
-- Per-app rules (rules.lua), kept in ~/.config/kwinish/rules.json and applied a moment after a window opens.
local function cfg_dir() return (os.getenv("XDG_CONFIG_HOME") or ((os.getenv("HOME") or "") .. "/.config")) .. "/kwinish" end

local function load_rules()
  local f = io.open(cfg_dir() .. "/rules.json", "r")
  local text = f and f:read("*a")
  if f then f:close() end
  return rules.parse(text)
end

local function save_rules(r)
  os.execute("mkdir -p '" .. cfg_dir() .. "'")
  local f = io.open(cfg_dir() .. "/rules.json", "w")
  if f then f:write(rules.serialize(r), "\n"); f:close() end
end

function M.apply_rules(w)
  local r = w and load_rules()[w.class]
  if not r then return end
  if r.noframe and not has_tag(w, "kwinish-noframe") then toggle_noframe(w) end
  if r.nocapture and not has_tag(w, "kwinish-nocapture") then toggle_nocapture(w) end
  if r.workspace then hl.dispatch(hl.dsp.window.move({ workspace = tostring(r.workspace), follow = false, window = "address:" .. w.address })) end
  if r.rect and w.floating then put(w, { x = r.rect[1], y = r.rect[2], w = r.rect[3], h = r.rect[4] }) end
  -- addresses are reused after a window closes, so trust the window's tag, not the table entry left by its predecessor
  if r.above and not has_tag(w, "kwinish-above") then above[w.address] = nil; M.keep_above({ window = w })
  elseif r.below and not has_tag(w, "kwinish-below") then below[w.address] = nil; M.keep_below({ window = w }) end
end

-- window_action "rule:<above|below|noframe|nocapture|workspace|rect|forget>": toggle that rule for the window's app.
local function window_rule(w, arg)
  local rs = load_rules()
  if arg == "forget" then rules.forget(rs, w.class)
  elseif rules.bools[arg] then rules.toggle(rs, w.class, arg, true)
  elseif arg == "workspace" then rules.toggle(rs, w.class, "workspace", w.workspace and w.workspace.id)
  elseif arg == "rect" and w.floating then
    local r = rect_of(w)
    rules.toggle(rs, w.class, "rect", { math.floor(r.x + 0.5), math.floor(r.y + 0.5), math.floor(r.w + 0.5), math.floor(r.h + 0.5) })
  else return end
  save_rules(rs)
  M.apply_rules(w) -- takes effect on this window too
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

local layout_rects -- defined below, with the layout state

-- Entry point for the window menu (kwinish-ui): action is "tile:left", "axis:h", "pack:up", "above", "float",
-- "fullscreen", "close" or "workspace:N", applied to the window with that address.
function M.window_action(addr, action)
  local kind, arg = action:match("^(%w+):?(.*)$")
  if kind == "layout" then M.set_layout(arg) return end -- needs no window
  local w = hl.get_window("address:" .. addr)
  if not w then return end
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
  elseif kind == "zone" then
    local rects = layout_rects(w.monitor)
    local r = rects[tonumber(arg) or 0]
    if r then place(w, r, "zone:" .. arg) end
  elseif kind == "grab" then
    -- "grab:move" / "grab:resize", needs the plugin. The window menu holds the keyboard until it quits and the compositor will not move
    -- focus meanwhile, so the grab started at once took whatever window was focused or under the pointer (the one in front, when
    -- the menu was opened on a covered window): wait for the menu to go
    hl.timer(function() plugin_call("grab", arg, addr) end, { timeout = 250, type = "oneshot" })
  elseif kind == "rule" then window_rule(w, arg)
  elseif kind == "mode" then if w.workspace then M.set_workspace_mode(w.workspace.id, arg) end
  elseif kind == "columns" then
    -- the window menu keeps the keyboard until it quits, and while it does the compositor will not move focus: wait for it to go
    local ws = w.workspace and w.workspace.id
    if ws then hl.timer(function() M.set_scrolling_columns(ws, arg, addr) end, { timeout = 250, type = "oneshot" }) end
  elseif kind == "pin" then hl.dispatch(hl.dsp.window.pin({ action = "toggle", window = sel }))
  elseif kind == "maximize" then hl.dispatch(hl.dsp.window.fullscreen({ mode = "maximized", window = sel }))
  elseif kind == "minimize" then hl.dispatch(hl.dsp.window.move({ workspace = "special:minimized", follow = false, window = sel }))
  elseif kind == "below" then M.keep_below({ window = w })
  elseif kind == "noframe" then toggle_noframe(w)
  elseif kind == "nocapture" then toggle_nocapture(w)
  end
end

-- Open the layout picker (kwinish-ui layouts) at the pointer.
function M.layouts() hl.exec_cmd("kwinish-ui layouts") end

-- Open the zone editor (kwinish-ui zones).
function M.zone_editor() hl.exec_cmd("kwinish-ui zones") end

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

-- Active tiling layout (a preset name from layouts.lua, or "off"), kept in ~/.config/kwinish/layout.
local function layout_file()
  local base = os.getenv("XDG_CONFIG_HOME") or ((os.getenv("HOME") or "") .. "/.config")
  return base .. "/kwinish", base .. "/kwinish/layout"
end

local function load_layout()
  local _, file = layout_file()
  local f = io.open(file, "r")
  if not f then return "grid" end
  local name = (f:read("*l") or ""):gsub("%s+", "")
  f:close()
  return (name == "off" or layouts.find(name)) and name or "grid"
end

local function load_user_layouts()
  local dir = layout_file()
  local f = io.open(dir .. "/layouts.json", "r")
  local ok = pcall(function() layouts.load_user(f and f:read("*a") or nil) end)
  if not ok then layouts.user = {} end
  if f then f:close() end
end
load_user_layouts()

M.layout = M.layout or load_layout()
M.layout_gap = M.layout_gap -- padding between layout zones; nil: the compositor's gaps_in

layout_rects = function(mon)
  local preset = layouts.find(M.layout)
  if not preset then return {} end
  local gap = preset.gap or M.layout_gap or num(hl.get_config("general.gaps_in"), "left")
  return layouts.zone_rects(preset.zones, work_area(mon), gap)
end

function M.layout_names()
  local out = { { name = "off", label = "Off (screen edges only)" } }
  for _, list in ipairs({ layouts.presets, layouts.user }) do
    for _, p in ipairs(list) do out[#out + 1] = { name = p.name, label = p.label } end
  end
  return out
end

function M.set_layout(name)
  load_user_layouts() -- the editor may have just saved
  if name ~= "off" and not layouts.find(name) then return end
  M.layout = name
  local dir, file = layout_file()
  os.execute("mkdir -p '" .. dir .. "'")
  local f = io.open(file, "w")
  if f then f:write(name, "\n"); f:close() end
  M.publish_zones()
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
    -- the active layout's zones, shown while Shift is held during a drag
    for i, r in ipairs(layout_rects(mon)) do
      pcall(p.set_zone, mon.name, "z:" .. i, r.x, r.y - bh, r.w, r.h + bh)
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

-- Workspace modes (modes.lua), kept in ~/.config/kwinish/workspace-modes.json. Tiling and scrolling are Hyprland's own layouts, set per
-- workspace with a workspace rule; floating floats the windows. Windows the mode floated carry the tag kwinish-mode-floated, so ending
-- the mode tiles only those, and the tag survives config reloads (a table here would not). Windows that already floated, are
-- fullscreen or pinned are left alone. A window's last floating rectangle is kept in a tag and
-- put back when the mode returns (see saved_rect).
local mode_cascade = 0

local function load_modes()
  local f = io.open(cfg_dir() .. "/workspace-modes.json", "r")
  local text = f and f:read("*a")
  if f then f:close() end
  return modes.parse(text)
end

local function save_modes(t)
  os.execute("mkdir -p '" .. cfg_dir() .. "'")
  local f = io.open(cfg_dir() .. "/workspace-modes.json", "w")
  if f then f:write(modes.serialize(t), "\n"); f:close() end
end

-- A window's floating rectangle is kept in a tag (kwinish-rect-X-Y-W-H, X and Y shifted by 10000 to stay positive) when the mode
-- ends, and put back when it begins again: Hyprland itself does not return a re-floated window to where it was.
local function saved_rect(w)
  for _, t in ipairs(w.tags or {}) do
    local x, y, ww, hh = t:match("^kwinish%-rect%-(%d+)%-(%d+)%-(%d+)%-(%d+)$")
    if x then return { x = tonumber(x) - 10000, y = tonumber(y) - 10000, w = tonumber(ww), h = tonumber(hh) } end
  end
end

local function save_rect(w)
  for _, t in ipairs(w.tags or {}) do
    if t:match("^kwinish%-rect%-") then set_tag(w, t, false) end
  end
  local r = rect_of(w)
  set_tag(w, string.format("kwinish-rect-%d-%d-%d-%d", math.floor(r.x + 10000.5), math.floor(r.y + 10000.5), math.floor(r.w + 0.5), math.floor(r.h + 0.5)), true)
end

local function float_for_mode(w)
  if w.floating or (w.fullscreen or 0) ~= 0 or w.pinned then return end
  local sel = "address:" .. w.address
  set_tag(w, "kwinish-mode-floated", true)
  hl.dispatch(hl.dsp.window.float({ action = "enable", window = sel }))
  local r = saved_rect(w)
  if r then put(w, r)
  elseif w.monitor then
    put(w, modes.open_rect(work_area(w.monitor), mode_cascade))
    mode_cascade = (mode_cascade + 1) % 5
  end
end

local function mode_of(ws, saved)
  local m = saved[tostring(ws)]
  if m then return m end
  for _, w in ipairs(hl.get_workspaces()) do
    if w.id == ws then return modes.from_engine(w.tiled_layout) end
  end
  return "tiling"
end

-- Applies `mode` to workspace `ws` (a number >= 1) now, and remembers it.
function M.set_workspace_mode(ws, mode)
  ws = tonumber(ws)
  if not ws or ws < 1 or not modes.valid[mode] then return end
  local saved = load_modes()
  saved[tostring(ws)] = mode
  save_modes(saved)
  hl.workspace_rule({ workspace = tostring(ws), layout = modes.engine(mode) })
  for _, w in ipairs(hl.get_workspace_windows(ws)) do
    if mode == "floating" then float_for_mode(w)
    elseif has_tag(w, "kwinish-mode-floated") then
      set_tag(w, "kwinish-mode-floated", false)
      if w.floating then save_rect(w) end
      if not w.pinned then hl.dispatch(hl.dsp.window.float({ action = "disable", window = "address:" .. w.address })) end
    end
  end
end

-- Next (or previous) mode for the workspace of the active window.
function M.cycle_workspace_mode(back)
  local w = hl.get_active_window()
  if not (w and w.workspace) then return end
  M.set_workspace_mode(w.workspace.id, modes.next(mode_of(w.workspace.id, load_modes()), back))
end

-- Scrolling workspace: make every column 1/n of the screen (n = 1, 2 or 3). Hyprland's colresize acts on the focused column, so each
-- window is focused in turn and the original focus comes back.
local function resize_columns(ws, n, keep)
  local active = keep and hl.get_window("address:" .. keep) or hl.get_active_window()
  local home = hl.get_active_workspace and hl.get_active_workspace() -- focusing a window of another workspace may switch to it
  local width = string.format("%.3f", 1 / n - (n > 1 and 0.001 or 0))
  for _, w in ipairs(hl.get_workspace_windows(ws)) do
    if not w.floating and (w.fullscreen or 0) == 0 then
      hl.dispatch(hl.dsp.focus({ window = "address:" .. w.address }))
      local now = hl.get_active_window()
      -- colresize acts on the focused column: when focus was refused (an overlay still holds the keyboard) skip, or it would hit another window
      if now and now.address == w.address then hl.dispatch(hl.dsp.layout("colresize " .. width)) end
    end
  end
  if active then hl.dispatch(hl.dsp.focus({ window = "address:" .. active.address })) end
  if home and home.id and (not active or not active.workspace or active.workspace.id ~= home.id) then
    hl.dispatch(hl.dsp.focus({ workspace = home.id })) -- back where the user was, also when no window was active
  end
end

-- workspace-columns.json remembers n per workspace: while it is set, the columns are redrawn to that width whenever a window opens or
-- closes there. Choosing the same n again clears it (columns are left alone again).
local function load_columns()
  local f = io.open(cfg_dir() .. "/workspace-columns.json", "r")
  local text = f and f:read("*a")
  if f then f:close() end
  return modes.parse_columns(text)
end

function M.set_scrolling_columns(ws, n, keep) -- keep: address of the window that ends up focused (the one the menu was opened on)
  ws, n = tonumber(ws), tonumber(n)
  if not ws or ws < 1 or not n or n < 1 or n > 3 then return end
  local cols = load_columns()
  if cols[tostring(ws)] == n then cols[tostring(ws)] = nil else cols[tostring(ws)] = n end
  os.execute("mkdir -p '" .. cfg_dir() .. "'")
  local f = io.open(cfg_dir() .. "/workspace-columns.json", "w")
  if f then f:write(modes.serialize_columns(cols), "\n"); f:close() end
  if cols[tostring(ws)] then
    if load_modes()[tostring(ws)] ~= "scrolling" then
      M.set_workspace_mode(ws, "scrolling")
      -- the layout switch settles a moment later; resizing columns before that finds none
      hl.timer(function()
        if load_columns()[tostring(ws)] == n and load_modes()[tostring(ws)] == "scrolling" then resize_columns(ws, n, keep) end
      end, { timeout = 300, type = "oneshot" })
    else
      resize_columns(ws, n, keep)
    end
  end
end

-- Re-applies the remembered width on a workspace after a window opened or closed there (debounced: one pass per burst).
local columns_pending = {}
function M.refresh_columns(ws)
  ws = tonumber(ws)
  if not ws or columns_pending[ws] then return end
  local n = load_columns()[tostring(ws)]
  if not n then return end
  columns_pending[ws] = true
  hl.timer(function()
    columns_pending[ws] = nil
    local m = load_modes()[tostring(ws)]
    if m == "scrolling" then resize_columns(ws, n) end
  end, { timeout = 250, type = "oneshot" })
end

-- A window that opens on a floating workspace floats too.
function M.apply_workspace_mode(w)
  local ws = w and w.workspace and w.workspace.id
  if ws and ws >= 1 and load_modes()[tostring(ws)] == "floating" then float_for_mode(w) end
end

-- After a config reload the workspace rules are gone: set them again.
for ws, mode in pairs(load_modes()) do hl.workspace_rule({ workspace = ws, layout = modes.engine(mode) }) end

-- Window shortcuts (shortcuts.json): one bind per app that focuses it. Binds are added only while the config loads
-- (hl.bind from a running eval can crash Hyprland 0.56.2); the guard also keeps a core reload by eval from re-binding.
if not KWINISH_SHORTCUTS then
  KWINISH_SHORTCUTS = true
  local f = io.open(cfg_dir() .. "/shortcuts.json", "r")
  local text = f and f:read("*a")
  if f then f:close() end
  -- `kwinish-ui shortcut` enters this empty submap while it listens for keys, so no bind runs instead of being captured
  -- (a submap with no bind in it is never registered, hence the bind on a key combination nobody types)
  hl.define_submap("kwcapture", function() hl.bind("CTRL + ALT + SHIFT + SUPER + F24", function() end) end)
  for class, combo in pairs(rules.parse_shortcuts(text)) do
    hl.bind(combo, function()
      hl.dispatch(hl.dsp.focus({ window = "class:" .. class }))
      local w = hl.get_window("class:" .. class)
      if w and w.floating then hl.dispatch(hl.dsp.window.alter_zorder({ mode = "top", window = "address:" .. w.address })) end
    end)
  end
end

if not KWINISH_RULES_HOOK then
  KWINISH_RULES_HOOK = true
  hl.on("window.active", function(w)
    local addr = w and w.address
    if not addr then return end
    hl.timer(function()
      local win = hl.get_window("address:" .. addr)
      local home = hl.get_active_workspace and hl.get_active_workspace()
      if win and win.workspace and win.workspace.name == "special:minimized" and home and home.id and home.id >= 1 then
        hl.dispatch(hl.dsp.window.move({ workspace = tostring(home.id), window = "address:" .. addr }))
        hl.dispatch(hl.dsp.window.alter_zorder({ mode = "top", window = "address:" .. addr }))
        hl.dispatch(hl.dsp.focus({ window = "address:" .. addr }))
      end
    end, { timeout = 30, type = "oneshot" })
  end)
  -- a window carried to another workspace leaves the mode that floated it: drop the claim, so that workspace's mode never tiles it
  hl.on("window.move_to_workspace", function(w)
    if w and w.address then
      local win = hl.get_window("address:" .. w.address)
      if win and has_tag(win, "kwinish-mode-floated") then set_tag(win, "kwinish-mode-floated", false) end
      local kw = package.loaded.kwinish
      if win and kw then
        kw.apply_workspace_mode(win) -- floating destination: the window floats too
        if win.workspace then kw.refresh_columns(win.workspace.id) end
      end
    end
  end)
  hl.on("window.close", function(w)
    local ws = w and w.workspace and w.workspace.id
    local kw = package.loaded.kwinish
    if w and w.address and kw then kw.forget_window(w.address) end
    if ws and kw then kw.refresh_columns(ws) end
  end)
  hl.on("window.open", function(w)
    local addr = w.address -- re-fetched: the window may be gone, and needs a moment to map
    hl.timer(function()
      local win = hl.get_window("address:" .. addr)
      local kw = package.loaded.kwinish
      if win and win.mapped and kw then kw.apply_workspace_mode(win); kw.apply_rules(win); if win.workspace then kw.refresh_columns(win.workspace.id) end end
    end, { timeout = 150, type = "oneshot" })
  end)
end

M._work_area, M._rect_of = work_area, rect_of -- exposed for spikes and tests
return M
