-- Integration tests with a plain-Lua compositor model; no live Hyprland or plugins.
package.path = (arg[0]:match("(.*/)") or "./") .. "?.lua;" .. package.path
local getenv, execute, open = os.getenv, os.execute, io.open
local pipe = assert(io.popen("mktemp -d /tmp/kwinish-test.XXXXXXXX", "r"))
local tmp = assert(pipe:read("*l"))
assert(pipe:close())
assert(tmp:match("^/tmp/kwinish%-test%.%w+$"))

-- Lua has no setenv: redirect the core's environment lookup, and reject any I/O
-- outside this directory (including reads of the user's real state).
os.getenv = function(key)
  if key == "HOME" then return tmp end
  if key:match("^XDG_.*_HOME$") or key == "XDG_RUNTIME_DIR" then return tmp .. "/" .. key end
  return getenv(key)
end
local failing_write -- while set, writes to any file opened for writing under a path containing it fail (a full disk)
io.open = function(path, mode)
  assert(type(path) == "string" and path:sub(1, #tmp + 1) == tmp .. "/" and not path:find("..", 1, true), "state escaped temporary directory")
  local f, err, code = open(path, mode)
  if f and failing_write and (mode or "r"):match("[wa]") and path:find(failing_write, 1, true) then
    return { write = function() return nil, "No space left on device" end, close = function() return f:close() end }
  end
  return f, err, code -- the errno too, as io.open gives it
end
os.execute = function(cmd)
  assert(cmd == "mkdir -p '" .. tmp .. "/XDG_CONFIG_HOME/kwinish'" or cmd == "mkdir -p '" .. tmp .. "/XDG_RUNTIME_DIR/kwinish'", "unexpected core command: " .. cmd)
  return execute(cmd)
end

local warps_log = {}
local function run()
  local mon = { name = "TEST", x = 0, y = 0, width = 1600, height = 1000, scale = 1, reserved = {} }
  local side = { name = "SIDE", x = 1600, y = 0, width = 1000, height = 800, scale = 1, reserved = {} }
  local workspaces = { { id = 1, name = "1", tiled_layout = "dwindle", monitor = mon }, { id = 2, name = "2", tiled_layout = "dwindle", monitor = mon },
    { id = -99, name = "special:minimized" }, { id = 4, name = "4", tiled_layout = "dwindle", monitor = side } }
  local wins, events, timers, calls, execs, rules = {}, {}, {}, {}, {}, {}
  local clock, serial, active, home, refused = 0, 0, nil, workspaces[1], nil
  local function window(addr, ws, floating)
    local w = { address = addr, class = "Test", title = addr, mapped = true, hidden = false, fullscreen = 0,
      floating = floating or false, monitor = mon, workspace = workspaces[ws or 1], at = { 40, 50 }, size = { 400, 300 }, tags = {} }
    wins[#wins + 1] = w
    return w
  end
  local function lookup(sel)
    for _, w in ipairs(wins) do if sel == "address:" .. w.address then return w end end
  end
  local function ws_lookup(id)
    for _, ws in ipairs(workspaces) do if tostring(ws.id) == tostring(id) or ws.name == id then return ws end end
    error("unknown workspace " .. tostring(id))
  end
  local function tag(w, name)
    for _, t in ipairs(w.tags) do if t == name then return true end end
    return false
  end
  local function emit(name, w) for _, fn in ipairs(events[name] or {}) do fn(w) end end
  local function advance(ms)
    local deadline = clock + ms
    while true do
      local idx
      for i, t in ipairs(timers) do
        if t.at <= deadline and (not idx or t.at < timers[idx].at or (t.at == timers[idx].at and t.id < timers[idx].id)) then idx = i end
      end
      if not idx then break end
      local t = table.remove(timers, idx)
      clock = t.at
      t.fn()
    end
    clock = deadline
  end
  local function command(kind)
    return function(args) return { kind = kind, args = args } end
  end
  hl = {
    dsp = { window = {}, focus = command("focus"), layout = command("layout") },
    get_monitors = function() return { mon, side } end,
    get_active_monitor = function() return mon end,
    get_workspace = function(sel)
      for _, ws in ipairs(workspaces) do if tostring(ws.id) == tostring(sel) then return ws end end
    end,
    -- Named rules: declaring a name again replaces it, enabled = false switches it off (Hyprland 0.56, checked nested).
    window_rule = function(spec) assert(spec.name, "rules are named"); rules[spec.name] = spec end,
    get_windows = function() return wins end,
    get_window = lookup,
    get_workspaces = function() return workspaces end,
    get_workspace_windows = function(id)
      local out = {}
      for _, w in ipairs(wins) do if w.workspace.id == id then out[#out + 1] = w end end
      return out
    end,
    get_active_window = function() return active end,
    get_active_workspace = function() return home end,
    get_config = function(key)
      assert(key == "general.gaps_out" or key == "general.gaps_in" or key == "cursor.no_warps", key)
      if key == "cursor.no_warps" then return false end
      return 0
    end,
    config = function(t) if t.cursor then warps_log[#warps_log + 1] = t.cursor.no_warps end end,
    get_loaded_plugins = function() return {} end,
    on = function(name, fn) events[name] = events[name] or {}; table.insert(events[name], fn) end,
    timer = function(fn, opts)
      assert(opts.type == "oneshot" and opts.timeout > 0, "unsupported timer")
      serial = serial + 1
      timers[#timers + 1] = { fn = fn, at = clock + opts.timeout, id = serial }
    end,
    workspace_rule = function(rule) ws_lookup(rule.workspace).tiled_layout = rule.layout end,
    exec_cmd = function(cmd) execs[#execs + 1] = cmd end,
    define_submap = function(name, fn) assert(name == "kwcapture"); fn() end,
    bind = function(keys, fn) assert(type(keys) == "string" and type(fn) == "function") end,
  }
  for _, kind in ipairs({ "float", "tag", "move", "resize", "alter_zorder" }) do hl.dsp.window[kind] = command(kind) end
  hl.dispatch = function(cmd)
    local a, kind = cmd.args, cmd.kind
    local w = type(a) == "table" and a.window and assert(lookup(a.window))
    calls[#calls + 1] = { kind = kind, args = a, address = w and w.address }
    if kind == "focus" then
      if w and w ~= refused then
        active = w
        if w.workspace.id >= 1 then home = w.workspace end
      elseif a.workspace then home = ws_lookup(a.workspace) end
    elseif kind == "layout" then
      assert(active and not active.floating and active.fullscreen == 0, "column resize needs a tiled focus")
      calls[#calls].address = active.address
      active.column = a
    elseif kind == "float" then
      assert(a.action == "enable" or a.action == "disable")
      w.floating = a.action == "enable"
      -- Tiling destroys floating geometry, so restoration must use the tag.
      w.at, w.size = { 0, 0 }, { 800, 1000 }
    elseif kind == "tag" then
      local name = a.tag:sub(2)
      local tags = {}
      for _, t in ipairs(w.tags) do if t ~= name then tags[#tags + 1] = t end end
      if a.tag:sub(1, 1) == "+" then tags[#tags + 1] = name end
      w.tags = tags
    elseif kind == "resize" then assert(a.exact); w.size = { a.x, a.y }
    elseif kind == "move" then
      if a.workspace then w.workspace = ws_lookup(a.workspace) else w.at = { a.x, a.y } end
    elseif kind == "alter_zorder" then w.zorder = a.mode
    else error("unsupported dispatch " .. tostring(kind)) end
  end
  -- A window opening: enabled rules for its workspace act before the tiled layout (match.float = false skips windows that float
  -- on their own); move is monitor-local and a rule's tag shows as "name*". Without a rule it is tiled over the whole area.
  local function open(addr, ws, natural_float)
    local w = window(addr, ws, natural_float)
    w.monitor = w.workspace.monitor or mon
    w.at, w.size = natural_float and { 650, 425 } or { w.monitor.x, w.monitor.y }, natural_float and { 300, 150 } or { w.monitor.width, w.monitor.height }
    for _, r in pairs(rules) do
      local m = r.match
      if r.enabled ~= false and m and tostring(m.workspace) == tostring(w.workspace.id) and (m.float == nil or m.float == w.floating) then
        if r.float then w.floating = true end
        if r.size then local x, y = r.size:match("^(%d+) (%d+)$"); w.size = { tonumber(x), tonumber(y) } end
        if r.move then local x, y = r.move:match("^(%-?%d+) (%-?%d+)$"); w.at = { w.monitor.x + tonumber(x), w.monitor.y + tonumber(y) } end
        if r.tag then w.tags[#w.tags + 1] = r.tag:sub(2) .. "*" end
      end
    end
    w.first = { floating = w.floating, x = w.at[1], y = w.at[2], w = w.size[1], h = w.size[2] }
    emit("window.open", w)
    return w
  end
  local function close(w)
    for i, x in ipairs(wins) do if x == w then table.remove(wins, i) break end end
  end
  local function count(kind)
    local n = 0
    for _, c in ipairs(calls) do if c.kind == kind then n = n + 1 end end
    return n
  end
  local function rect(w, x, y, width, height)
    assert(w.at[1] == x and w.at[2] == y and w.size[1] == width and w.size[2] == height, "wrong rectangle for " .. w.address)
  end
  local function state(file)
    local f = assert(io.open(tmp .. "/XDG_CONFIG_HOME/kwinish/" .. file, "r"))
    local text = f:read("*a"); f:close(); return text
  end
  local function reload()
    package.loaded.kwinish = nil
    return require("kwinish") -- retain compositor windows, tags, and global hook guards
  end
  local a, b = window("a"), window("b")
  local manual, full, pinned = window("manual", 1, true), window("full"), window("pinned")
  full.fullscreen, pinned.pinned = 1, true
  local other = window("other", 2)
  active = a
  local kw = reload()
  -- Keyboard snapping uses the active floating window without options.
  active = manual
  kw.tile("left")
  rect(manual, 0, 0, 800, 1000)
  kw.tile("left")
  rect(manual, 40, 50, 400, 300)
  active = a
  kw.set_workspace_mode(1, "tiling")
  assert(home.tiled_layout == "dwindle" and not a.floating)
  kw.set_workspace_mode("1", "scrolling")
  assert(home.tiled_layout == "scrolling" and not b.floating)
  kw.set_workspace_mode(1, "floating")
  assert(home.tiled_layout == "dwindle" and a.floating and b.floating and tag(a, "kwinish-mode-floated"))
  rect(a, 240, 112, 1024, 680); rect(b, 264, 136, 1024, 680)
  assert(manual.floating and not tag(manual, "kwinish-mode-floated") and not full.floating and not pinned.floating and not other.floating, "mode exclusions")
  a.at, a.size = { -120, 230 }, { 510, 420 }
  b.at, b.size = { 700, -80 }, { 620, 330 }
  kw.set_workspace_mode(1, "tiling")
  assert(not a.floating and not b.floating and manual.floating and not tag(a, "kwinish-mode-floated"))
  assert(tag(a, "kwinish-rect-9880-10230-510-420") and tag(b, "kwinish-rect-10700-9920-620-330"), "per-window rectangles encoded in tags")
  assert(require("modes").parse(state("workspace-modes.json"))["1"] == "tiling")
  home.tiled_layout = "scrolling" -- compositor rules disappear on config reload
  kw = reload()
  assert(home.tiled_layout == "dwindle", "workspace rule restored on reload")
  kw.set_workspace_mode(1, "floating")
  rect(a, -120, 230, 510, 420); rect(b, 700, -80, 620, 330)
  a.at = { 25, 35 }
  kw.set_workspace_mode(1, "tiling")
  assert(tag(a, "kwinish-rect-10025-10035-510-420") and not tag(a, "kwinish-rect-9880-10230-510-420"), "old rectangle tag replaced")
  kw.set_workspace_mode(1, "floating"); rect(a, 25, 35, 510, 420)
  kw.set_workspace_mode(1, "tiling")
  assert(rules["kwinish-mode-floating-1"] and rules["kwinish-mode-floating-1"].enabled == false, "leaving floating switches the rule off")

  -- A window that opens on a floating workspace is born floating at the mode's rectangle: floating it after it opened showed it at
  -- its tiled size first. `other` takes cascade step 0, so the rule waits at step 1.
  kw.set_workspace_mode(2, "floating")
  rect(other, 240, 112, 1024, 680)
  local n1 = open("n1", 2)
  local f1 = n1.first
  assert(f1.floating and f1.x == 264 and f1.y == 136 and f1.w == 1024 and f1.h == 680, "born floating at the mode rectangle")
  assert(tag(n1, "kwinish-mode-floated"), "claimed by the mode")
  local n2 = open("n2", 2)
  assert(n2.first.floating and n2.first.x == 288 and n2.first.y == 160, "the next window cascades from its first frame")
  local dlg = open("dlg", 2, true)
  assert(dlg.size[1] == 300 and dlg.at[1] == 650 and not tag(dlg, "kwinish-mode-floated"), "a window that floats on its own is left alone")
  calls = {}
  advance(150) -- the 150 ms pass that used to float them must not place them again
  assert(count("resize") == 0 and count("move") == 0 and n1.at[1] == 264 and n2.at[1] == 288, "no second placement")
  -- A workspace on a monitor away from the origin: the rule's move is relative to that monitor.
  kw.set_workspace_mode(4, "floating")
  local n3 = open("n3", 4)
  assert(n3.first.floating and n3.first.x == 1804 and n3.first.y == 152 and n3.first.w == 640 and n3.first.h == 544,
    "rect on the side monitor: " .. n3.first.x .. "," .. n3.first.y .. " " .. n3.first.w .. "x" .. n3.first.h)
  -- Leaving the mode tiles the windows born floating, keeps the one that floats on its own, and new windows tile again.
  kw.set_workspace_mode(2, "tiling")
  assert(not n1.floating and not n2.floating and dlg.floating and not other.floating, "born-floating windows go back to tiling")
  assert(rules["kwinish-mode-floating-2"].enabled == false and not open("n4", 2).first.floating, "rule off: new windows tile")
  -- The compositor drops rules on a config reload: loading the core declares them again, or monitor.added if no monitor exists yet.
  rules = {}
  kw = reload()
  assert(rules["kwinish-mode-floating-4"] and rules["kwinish-mode-floating-4"].enabled ~= false and not rules["kwinish-mode-floating-2"], "floating rule back after reload")
  rules = {}
  local active_monitor, side_ws = hl.get_active_monitor, workspaces[4]
  hl.get_active_monitor, side_ws.monitor = function() return nil end, nil
  kw = reload()
  assert(next(rules) == nil, "no monitor yet: no rule, no error")
  hl.get_active_monitor, side_ws.monitor = active_monitor, side
  emit("monitor.added", side)
  assert(rules["kwinish-mode-floating-4"], "rule declared once a monitor appears")
  side.width = 1200; rules["kwinish-mode-floating-4"] = nil
  emit("monitor.layout_changed", side)
  assert(rules["kwinish-mode-floating-4"] and rules["kwinish-mode-floating-4"].size == "768 544", "rule follows a monitor resize")
  side.width = 1000
  kw.set_workspace_mode(4, "tiling")
  for _, w in ipairs({ n1, n2, n3, dlg, wins[#wins] }) do close(w) end

  -- Switching layout defers resizing for 300 ms; only eligible columns change.
  calls = {}
  active = b
  kw.set_scrolling_columns(1, 2, a.address)
  assert(home.tiled_layout == "scrolling" and count("layout") == 0)
  advance(299); assert(count("layout") == 0, "layout must settle first")
  advance(1)
  assert(count("layout") == 3 and a.column == "colresize 0.499" and b.column == a.column and pinned.column == a.column)
  assert(manual.column == nil and full.column == nil and other.column == nil and active == a, "resize eligibility and explicit focus restoration")
  assert(require("modes").parse_columns(state("workspace-columns.json"))["1"] == 2)
  calls = {}
  kw.refresh_columns(1); kw.refresh_columns(1); kw.refresh_columns("1")
  advance(249); assert(count("layout") == 0)
  advance(1); assert(count("layout") == 3 and active == a, "refresh debounced and focus restored")
  kw.refresh_columns(1); advance(250); assert(count("layout") == 6, "debounce rearms")
  kw.set_scrolling_columns(1, 2)
  assert(next(require("modes").parse_columns(state("workspace-columns.json"))) == nil, "same choice clears remembered columns")
  calls = {}; kw.refresh_columns(1); advance(500); assert(count("layout") == 0)
  kw.set_scrolling_columns(1, 3)
  assert(a.column == "colresize 0.332", "already scrolling resizes immediately")
  calls = {}; refused = b
  kw.set_scrolling_columns(1, 1)
  assert(count("layout") == 2 and a.column == "colresize 1.000", "refused focus skips the column")
  refused = nil
  active, home = other, workspaces[2]
  kw.refresh_columns(1); advance(250)
  assert(active == other and home == workspaces[2], "cross-workspace refresh restores focus and workspace")
  active = nil
  kw.refresh_columns(1); advance(250)
  assert(home == workspaces[2], "workspace restored when no window was active")
  kw.set_workspace_mode(1, "tiling"); calls = {}
  kw.set_scrolling_columns(1, 2); kw.set_scrolling_columns(1, 2)
  advance(300); assert(count("layout") == 0, "cleared setting cancels delayed resize")
  kw.set_workspace_mode(1, "tiling")
  kw.set_scrolling_columns(1, 3); kw.set_workspace_mode(1, "tiling")
  advance(300); assert(count("layout") == 0, "changed mode cancels delayed resize")
  kw.refresh_columns(1); advance(250); assert(count("layout") == 0, "refresh respects current mode")

  -- Tags are authoritative for toggles and reconstruct the stack after reload.
  kw.keep_above({ window = manual })
  assert(tag(manual, "kwinish-above") and manual.zorder == "top")
  kw.keep_below({ window = manual })
  assert(tag(manual, "kwinish-below") and not tag(manual, "kwinish-above") and manual.zorder == "bottom")
  kw.keep_above({ window = manual })
  assert(not tag(manual, "kwinish-below") and tag(manual, "kwinish-above"))
  kw.keep_below({ window = b }); kw.keep_below({ window = b })
  assert(not tag(b, "kwinish-below"), "below toggles off")
  kw.keep_below({ window = b })
  kw = reload(); kw.setup()
  manual.zorder, b.zorder = nil, nil
  active = a; emit("window.active", a)
  advance(29); assert(manual.zorder == nil and b.zorder == nil)
  advance(1); assert(manual.zorder == "top" and b.zorder == "bottom", "stack reconstructed from tags after reload")
  kw.keep_above({ window = manual })
  assert(not tag(manual, "kwinish-above"), "above toggle reads surviving tag")
  manual.zorder = nil
  emit("window.active", a); advance(30); assert(manual.zorder == nil, "disabled above no longer raised")

  -- Unchecking a rule takes the switch off the open window too
  manual.class = "RuleApp"
  kw.window_action(manual.address, "rule:above"); assert(tag(manual, "kwinish-above"), "rule on applies to the open window")
  kw.window_action(manual.address, "rule:above"); assert(not tag(manual, "kwinish-above"), "rule off undoes it on the open window")
  kw.window_action(manual.address, "rule:below"); assert(tag(manual, "kwinish-below"))
  kw.window_action(manual.address, "rule:forget"); assert(not tag(manual, "kwinish-below"), "forgetting the app undoes its switches")

  kw.window_action(manual.address, "minimize")
  assert(manual.workspace.name == "special:minimized")
  home, active = workspaces[2], manual
  emit("window.active", manual)
  advance(29); assert(manual.workspace.name == "special:minimized")
  advance(1)
  assert(manual.workspace == workspaces[2] and active == manual and manual.zorder == "top", "minimized window restored, raised and focused on current workspace")
  -- Gone windows and an absent regular workspace must be harmless.
  emit("window.active", { address = "gone" }); advance(30)
  kw.window_action(manual.address, "minimize")
  home = workspaces[3]; emit("window.active", manual); advance(30)
  assert(manual.workspace.name == "special:minimized", "no restoration into a special workspace")

  -- Snap Assist: filling a zone offers the loose floating windows of the workspace for the next empty zone.
  kw.set_layout("halves")
  local c, d, e = window("c", 2, true), window("d", 2, true), window("e", 2, true)
  c.at, d.at, e.at = { 900, 900 }, { 1000, 900 }, { 1100, 900 }
  local function assist_json() local f = assert(io.open(tmp .. "/XDG_RUNTIME_DIR/kwinish/assist.json", "r")); local t = f:read("*a"); f:close(); return t end
  kw.window_action(c.address, "zone:1")
  assert(#execs == 1 and execs[1] == "flock '" .. tmp .. "/XDG_RUNTIME_DIR/kwinish/assist.lock' env KWINISH_ASSIST_GENERATION=1 kwinish-ui assist", "assist opened")
  local j = assist_json()
  assert(j:find('"monitor":"TEST","zone":2,', 1, true) and j:find('"address":"d"', 1, true) and j:find('"address":"e"', 1, true) and not j:find('"address":"c"', 1, true), j)
  local f = window("f", 2, true)
  kw.window_action(f.address, "zone:1") -- another snap before the overlay gets a choice
  assert(#execs == 2 and execs[2]:find("KWINISH_ASSIST_GENERATION=2 kwinish-ui assist", 1, true), "rapid snap gets its own serialized generation")
  assert(assist_json():find('"generation":2', 1, true), "latest generation published")
  kw.window_action(d.address, "zone:2") -- the last empty zone: nothing left to offer
  assert(#execs == 2, "no assist when every zone is filled")
  kw.window_action(d.address, "zone:2") -- same request again restores the window: no assist either
  assert(#execs == 2, "no assist on restore")
  kw.snap_assist = false
  kw.window_action(e.address, "zone:2"); assert(#execs == 2, "assist switched off")
  -- Dropped on another monitor: the zone and the half are computed on that monitor, not the window's own.
  local g = window("g", 2, true)
  kw.window_action(g.address, "zone:2@SIDE")
  assert(g.at[1] == 1600 + 500 and g.size[1] == 500, "zone on the other monitor: " .. g.at[1] .. " " .. g.size[1])
  local h = window("h", 2, true)
  kw.window_action(h.address, "tile:left@SIDE")
  assert(h.at[1] == 1600 and h.size[1] == 500, "half on the other monitor: " .. h.at[1] .. " " .. h.size[1])
  local k = window("k", 2, true)
  kw.window_action(k.address, "tile:left@NOPE") -- unknown monitor: falls back to the window's own
  assert(k.at[1] == 0, "fallback to own monitor")
  -- A plugin drop includes the monitor suffix even on the window's own monitor.
  kw.snap_assist = true
  kw.set_layout("thirds")
  local drop, candidate = window("drop", 2, true), window("candidate", 2, true)
  drop.at, candidate.at = { 900, 900 }, { 1100, 900 }
  candidate.monitor = hl.get_monitors()[2] -- a real candidate on the drop monitor exercises the cross-monitor guard
  local before_drop = #execs
  kw.window_action(drop.address, "zone:1@SIDE")
  rect(drop, 1600, 0, 333, 800)
  assert(#execs == before_drop, "cross-monitor drop must not open Snap Assist")
  kw.window_action(drop.address, "zone:1@SIDE") -- restore before testing the same zone on TEST
  candidate.monitor = drop.monitor
  kw.window_action(drop.address, "zone:1@TEST")
  assert(#execs == before_drop + 1 and execs[#execs]:find("kwinish-ui assist", 1, true),
    "same-monitor drop must open Snap Assist")

  -- settings.json: plugin options go to hl.config only while the plugin is loaded; snap_assist stays in the core
  local f = assert(io.open(tmp .. "/XDG_CONFIG_HOME/kwinish/settings.json", "w"))
  f:write('{"bar_height": 34, "snap_assist": false, "bar_color": "nope"}'); f:close()
  local plugin_cfg, config = {}, hl.config
  hl.config = function(t) if t.plugin then plugin_cfg[#plugin_cfg + 1] = t.plugin.kwinish else config(t) end end
  kw = reload()
  assert(#plugin_cfg == 0 and kw.snap_assist == false, "no plugin: nothing sent, core key applied")
  hl.plugin = { kwinish = {} }
  kw.apply_settings()
  hl.plugin, hl.config = nil, config
  assert(#plugin_cfg == 1 and plugin_cfg[1].bar_height == 34, "plugin loaded: valid keys sent, bad colour dropped")

  -- a file that is there but refused gives no entries, and one stderr line per reason however often it is read
  local said, stderr = {}, io.stderr
  io.stderr = { write = function(_, ...) said[#said + 1] = table.concat({ ... }) end }
  f = assert(io.open(tmp .. "/XDG_CONFIG_HOME/kwinish/rules.json", "w")); f:write('{"Test":{"above":true}'); f:close()
  local calls_before = #calls
  kw.apply_rules(a); kw.apply_rules(a)
  io.stderr = stderr
  assert(#calls == calls_before, "truncated rules.json applies nothing")
  assert(#said == 1 and said[1]:find("kwinish: rules.json ignored: ", 1, true), "refused file reported once: " .. table.concat(said, " | "))

  -- a layout zone narrower than its padding: never published to the plugin, and zone:N leaves the window where it is
  f = assert(io.open(tmp .. "/XDG_CONFIG_HOME/kwinish/layouts.json", "w"))
  f:write('[{"name":"user-tiny","gap":60,"zones":[[0,0,0.5,1],[0.5,0,0.01,1],[0.6,0,0.4,1]]}]'); f:close()
  local published = {}
  hl.plugin = { kwinish = { clear_zones = function() published = {} end,
    set_zone = function(mon_name, zone, x, y, w, h) if mon_name == "TEST" then published[zone] = { w = w, h = h } end end } }
  kw.set_layout("user-tiny")
  hl.plugin = nil
  assert(published["z:1"] and published["z:3"] and published["z:2"] == nil, "unusable zone not published")
  for zone, r in pairs(published) do assert(r.w > 0 and r.h > 0, "non-positive zone published: " .. zone) end
  local loose = window("loose", 1, true)
  kw.window_action(loose.address, "zone:2")
  rect(loose, 40, 50, 400, 300)
  kw.window_action(loose.address, "zone:3")
  assert(loose.size[1] > 0 and loose.at[1] > 900, "usable zone still works")

  -- a failed write leaves the old bytes and no temporary file, and is reported (rules, modes, columns, active layout)
  local cfg = tmp .. "/XDG_CONFIG_HOME/kwinish/"
  local function put(file, text) local h = assert(io.open(cfg .. file, "w")); h:write(text); h:close() end
  local before = { ["rules.json"] = '{"Test":{"above":true}}', ["workspace-modes.json"] = '{"2":"floating"}', ["workspace-columns.json"] = '{"2":2}', layout = "halves\n" }
  for file, text in pairs(before) do put(file, text) end
  said = {}
  io.stderr = { write = function(_, ...) said[#said + 1] = table.concat({ ... }) end }
  failing_write = cfg
  kw.window_action(a.address, "rule:below")
  kw.set_workspace_mode(2, "tiling")
  kw.set_scrolling_columns(2, 3)
  kw.set_layout("grid")
  failing_write = nil
  io.stderr = stderr
  for file, text in pairs(before) do
    assert(state(file) == text, file .. " lost its bytes to a failed write: " .. state(file))
    assert(io.open(cfg .. file .. ".tmp") == nil, file .. ": temporary file left")
  end
  assert(#said >= 4 and table.concat(said):find("kwinish: cannot write rules.json", 1, true), "failed writes reported: " .. table.concat(said, " | "))
  -- a rules.json the reader refuses is copied to rules.json.bak before it is replaced; without that copy nothing is replaced
  put("rules.json", '{"Test":{"above":true}')
  kw.window_action(a.address, "rule:below")
  assert(state("rules.json.bak") == '{"Test":{"above":true}', "refused rules.json kept as .bak")
  assert(require("rules").parse(state("rules.json")).Test.below, "then replaced")
  put("rules.json", "not json")
  io.stderr = { write = function() end }
  failing_write = "rules.json.bak"
  kw.window_action(a.address, "rule:below")
  failing_write, io.stderr = nil, stderr
  assert(state("rules.json") == "not json", "no copy, no replacement")
end

local ok, err = xpcall(run, debug.traceback)
os.getenv, os.execute, io.open = getenv, execute, open
assert(execute("rm -rf -- '" .. tmp .. "'"))
assert(ok, err)
assert(#warps_log >= 2 and warps_log[1] == true and warps_log[#warps_log] == false, "pointer warps restored after the column pass")
print("kwinish ok")
