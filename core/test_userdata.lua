-- Regressions for the readers of the user's JSON files (harden-user-data-io): hostile or broken text must give nothing, never run.
-- Code that could hang is only ever run in a child interpreter under `timeout`, never in this process (nor in Hyprland).
local dir = arg[0]:match("(.*/)") or "./"
package.path = dir .. "?.lua;" .. package.path
local rules, layouts, settings, modes = require("rules"), require("layouts"), require("settings"), require("modes")

local failed = 0
local function check(name, ok)
  print((ok and "ok - " or "FAIL - ") .. name)
  if not ok then failed = failed + 1 end
end
local function empty(t) return type(t) == "table" and next(t) == nil end

-- Parse `text` with module `mod`'s function `fn` in a child lua, at most 5 s. Returns "empty", "entries" or "timeout"/"error".
local function sh(s) return "'" .. s:gsub("'", "'\\''") .. "'" end
local function isolated(mod, fn, text)
  local file, child = os.tmpname(), os.tmpname()
  local f = assert(io.open(file, "w")); f:write(text); f:close()
  f = assert(io.open(child, "w"))
  f:write(string.format([[
package.path = %q .. "?.lua;" .. package.path
local M = require(%q)
local f = io.open(%q); local text = f:read("a"); f:close()
local r = M[%q](text)
if %q == "load_user" then r = M.user end
os.exit((type(r) == "table" and next(r) == nil) and 0 or 3)]], dir, mod, file, fn, fn))
  f:close()
  local _, how, n = os.execute("timeout 5 " .. sh(arg[-1]) .. " " .. sh(child))
  os.remove(file); os.remove(child)
  if how == "exit" and n == 0 then return "empty" elseif how == "exit" and n == 3 then return "entries" end
  return n == 124 and "timeout" or "error"
end

local loop = "(function() while true do end end)()"
local function hostile(name, mod, fn, text) local r = isolated(mod, fn, text); check(name .. ": Lua loop in the file ends, gives nothing (" .. r .. ")", r == "empty") end

-- Each reader: a Lua loop (child process), truncated text (an entry that looks complete is not applied) and a wrong root (the right
-- entries one level down are not the file's entries).
hostile("rules", "rules", "parse", '{"k":' .. loop .. '}')
check("rules: truncated", empty(rules.parse('{"kitty":{"above":true}')))
check("rules: array root", empty(rules.parse('[{"kitty":{"above":true}}]')))
check("rules: entry that is not an object", empty(rules.parse('{"kitty":[true]}')))

if failed > 0 then print(failed .. " failure(s)"); os.exit(1) end
print("userdata ok")
