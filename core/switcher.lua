-- Alt+Tab logic. Keys and focus stay here (Lua sees key releases through input.keyboard.key);
-- kwinish-ui only draws the state file. Nothing grabs the keyboard, so a stuck switcher cannot lock input.
local kwjson = require("kwjson")
local M = {}

local ALT_KEYS = { [64] = true, [108] = true } -- xkb keycodes (evdev + 8): Alt_L, Alt_R
local ESC = 9
local TIMEOUT_MS = 15000

local function state_file() return (os.getenv("XDG_RUNTIME_DIR") or "/tmp") .. "/kwinish/switcher.json" end

local function esc(s)
  return (tostring(s or ""):gsub('[%c"\\]', function(c) return string.format("\\u%04x", c:byte()) end))
end

-- Runtime state, disposable: published whole (kwjson.write, temporary file + rename) so kwinish-ui never draws half a file; no backup.
local function write(open, selected, items)
  local parts = {}
  for _, it in ipairs(items or {}) do
    parts[#parts + 1] = string.format('{"class":"%s","title":"%s"}', esc(it.class), esc(it.title))
  end
  kwjson.write(state_file(), string.format('{"open":%s,"selected":%d,"items":[%s]}', tostring(open), selected or 0, table.concat(parts, ",")))
end

local opts = {}
local mru = {}          -- addresses, most recently focused first
local open, items, sel, gen = false, {}, 0, 0

local function touch(addr)
  for i, a in ipairs(mru) do if a == addr then table.remove(mru, i) break end end
  table.insert(mru, 1, addr)
end

local function close(commit)
  if not open then return end
  open = false; gen = gen + 1
  local pick = items[sel + 1]
  write(false, 0, {})
  if commit and pick then
    hl.dispatch(hl.dsp.focus({ window = "address:" .. pick.address }))
  end
end

local function build()
  local list, seen = {}, {}
  local function add(w)
    -- skip unmapped windows and nameless ones (popups, tooltips, helper surfaces)
    if w.mapped and not seen[w.address] and (w.class ~= "" or w.title ~= "") and (not opts.filter or opts.filter(w)) then
      seen[w.address] = true
      list[#list + 1] = { address = w.address, class = w.class, title = w.title }
    end
  end
  for _, addr in ipairs(mru) do local w = hl.get_window("address:" .. addr); if w then add(w) end end
  for _, w in ipairs(hl.get_windows()) do add(w) end -- windows never focused since load
  return list
end

-- dir = 1 (forward) or -1 (backward); first call opens the overlay.
function M.step(dir)
  if not open then
    items = build()
    if #items < 2 then return end
    open = true
    sel = dir > 0 and 1 or #items - 1
    gen = gen + 1
    local my = gen
    hl.timer(function() if open and gen == my then close(false) end end, { timeout = TIMEOUT_MS, type = "oneshot" })
  else
    sel = (sel + dir) % #items
  end
  write(true, sel, items)
end

function M.setup(o)
  opts = o or {}
  local cur = hl.get_active_window()
  if cur then touch(cur.address) end
  hl.on("window.active", function(w) if w and not open then touch(w.address) end end)
  -- a closed window leaves the history (its address may be reused by another one)
  hl.on("window.close", function(w)
    if not (w and w.address) then return end
    for i, a in ipairs(mru) do if a == w.address then table.remove(mru, i) break end end
    if open then -- the selection on screen must not keep a window that is gone (its address may be reused)
      for i = #items, 1, -1 do if items[i].address == w.address then table.remove(items, i) end end
      if #items == 0 then close(false) else sel = math.min(sel, #items - 1); write(true, sel, items) end
    end
  end)
  hl.on("input.keyboard.key", function(code, _, state)
    if not open then return end
    if state == 0 and ALT_KEYS[code] then close(true)
    elseif state == 1 and code == ESC then close(false) end
  end)
end

M._state = function() return open, sel, #items end -- for tests
return M
