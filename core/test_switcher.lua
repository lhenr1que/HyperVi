package.path = (arg[0]:match("(.*/)") or "./") .. "?.lua;" .. package.path
-- The switcher writes $XDG_RUNTIME_DIR/kwinish/switcher.json, which a running kwinish-ui draws: keep it in a
-- temp dir even when this file is run by hand from a desktop session, and fail on any I/O outside it.
local real_getenv, real_execute, real_open = os.getenv, os.execute, io.open
local pipe = assert(io.popen("mktemp -d /tmp/kwinish-test.XXXXXXXX", "r"))
local tmp = assert(pipe:read("*l"))
assert(pipe:close())
assert(tmp:match("^/tmp/kwinish%-test%.%w+$"))
os.getenv = function(key) if key == "XDG_RUNTIME_DIR" then return tmp end return real_getenv(key) end
io.open = function(path, mode)
  assert(type(path) == "string" and path:sub(1, #tmp + 1) == tmp .. "/" and not path:find("..", 1, true), "state escaped temporary directory")
  return real_open(path, mode)
end
os.execute = function(cmd)
  assert(cmd == "mkdir -p '" .. tmp .. "/kwinish'", "unexpected switcher command: " .. cmd)
  return real_execute(cmd)
end

local wins = {
  a = { address = "a", class = "A", title = "one", mapped = true },
  b = { address = "b", class = "B", title = "two", mapped = true },
  c = { address = "c", class = "C", title = "three", mapped = true },
  g = { address = "g", class = "", title = "", mapped = true, xwayland = true },
  u = { address = "u", class = "U", title = "unmapped", mapped = false },
}
local focused, on_key, on_active = nil, nil, nil
hl = {
  get_active_window = function() return wins.a end,
  get_window = function(sel) return wins[sel:match("address:(.*)")] end,
  get_windows = function() return { wins.a, wins.b, wins.c, wins.g, wins.u } end,
  dsp = { focus = function(t) return t end },
  dispatch = function(t) focused = t.window end,
  timer = function() end,
  on = function(name, f) if name == "input.keyboard.key" then on_key = f else on_active = f end end,
}
local sw = require("switcher")
sw.setup()
sw.step(1)
local open, sel, n = sw._state(); assert(open and sel == 1 and n == 3, "one step selects the second window")
sw.step(1); open, sel = sw._state(); assert(sel == 2, "second step")
sw.step(1); open, sel = sw._state(); assert(sel == 0, "wraps")
sw.step(-1); open, sel = sw._state(); assert(sel == 2, "backwards")
on_key(9, 0, 1); open = sw._state(); assert(not open and focused == nil, "esc cancels without focusing")
sw.step(1); on_key(64, 0, 1); assert(sw._state(), "alt press does nothing")
on_key(64, 0, 0); open = sw._state(); assert(not open and focused == "address:b", "alt release commits, got " .. tostring(focused))
local f = assert(real_open(tmp .. "/kwinish/switcher.json", "r"), "state written to the temp runtime"); f:close()
os.getenv, os.execute, io.open = real_getenv, real_execute, real_open
assert(real_execute("rm -rf -- '" .. tmp .. "'"))
print("switcher ok")
