-- Minimal Hyprland 0.56.2 config. run.py supplies paths in case.lua.
NESTED_ROOT = "/tmp/kwinish-nested-open-x0q7jc80"
NESTED_CASE = "rules"
NESTED_REPO = "/home/ryo/Downloads/ryoku-work/kwinish-floating"
hl.monitor({ output = "", mode = "1280x800@60", position = "0x0", scale = 1 })
hl.config({
  general = { layout = "dwindle", gaps_in = 0, gaps_out = 0, border_size = 0 },
  animations = { enabled = false },
  misc = { disable_hyprland_logo = true, disable_splash_rendering = true },
  xwayland = { enabled = false },
})

local sequence = 0
local function json(v)
  if v == nil then return "null" end
  if type(v) ~= "string" then return tostring(v) end
  return '"' .. v:gsub('\\', '\\\\'):gsub('"', '\\"'):gsub('\n', '\\n'):gsub('\r', '\\r') .. '"'
end
local function vec(v)
  if not v then return "null" end
  return "[" .. json(v.x or v[1]) .. "," .. json(v.y or v[2]) .. "]"
end
local function log(event, w, detail)
  sequence = sequence + 1
  local fields = { '"seq":' .. sequence, '"event":' .. json(event) }
  if w then
    local values = { address = w.address, class = w.class, mapped = w.mapped, floating = w.floating,
      fullscreen = w.fullscreen, pinned = w.pinned, workspace = w.workspace and w.workspace.id,
      monitor = w.monitor and w.monitor.name }
    for k, v in pairs(values) do fields[#fields + 1] = json(k) .. ":" .. json(v) end
    fields[#fields + 1] = '"at":' .. vec(w.at)
    fields[#fields + 1] = '"size":' .. vec(w.size)
  end
  if detail then fields[#fields + 1] = '"detail":' .. json(detail) end
  local f = assert(io.open(NESTED_ROOT .. "/events.jsonl", "a"))
  f:write("{", table.concat(fields, ","), "}\n"); f:close()
end

hl.on("window.open_early", function(w)
  log("open_early.before", w)
  if NESTED_CASE == "early-mutate" then
    local ok, err = pcall(function()
      local selector = "address:" .. w.address
      hl.dispatch(hl.dsp.window.float({ action = "enable", window = selector }))
      hl.dispatch(hl.dsp.window.resize({ x = 819, y = 544, exact = true, window = selector }))
      hl.dispatch(hl.dsp.window.move({ x = 230, y = 128, window = selector }))
    end)
    log("open_early.after", w, ok and "dispatches accepted" or tostring(err))
  end
end)
hl.on("window.open", function(w) log("open", w) end)

-- Redirect Lua state I/O before loading the core, never the owner's config.
if NESTED_CASE == "core" then
  local getenv = os.getenv
  os.getenv = function(k)
    if k == "HOME" then return NESTED_ROOT .. "/home" end
    if k == "XDG_CONFIG_HOME" then return NESTED_ROOT .. "/config" end
    if k == "XDG_STATE_HOME" then return NESTED_ROOT .. "/state" end
    if k == "XDG_CACHE_HOME" then return NESTED_ROOT .. "/cache" end
    if k == "XDG_RUNTIME_DIR" then return NESTED_ROOT .. "/runtime" end
    return getenv(k)
  end
  package.path = NESTED_REPO .. "/core/?.lua;" .. package.path
  require("kwinish")
end
local f = assert(io.open(NESTED_ROOT .. "/ready", "w"))
f:write(os.getenv("HYPRLAND_INSTANCE_SIGNATURE") or ""); f:close()
