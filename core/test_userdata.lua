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

hostile("layouts", "layouts", "load_user", "[" .. loop .. "]")
layouts.load_user('[{"name":"a","zones":[[0,0,1,1]]}'); check("layouts: truncated", #layouts.user == 0)
layouts.load_user('{"name":"a","zones":[[0,0,1,1]]}'); check("layouts: object root", #layouts.user == 0)
layouts.load_user('[{"name":"a","made-by":"me","zones":[[0,0,1,1]]}]'); check("layouts: unknown member with a dash ignored", #layouts.user == 1)
layouts.load_user('[{"name":"a","label":"Left [wide]","zones":[[0,0,1,1]]}]')
check("layouts: brackets inside a label kept", layouts.user[1] and layouts.user[1].label == "Left [wide]")
layouts.load_user('[{"name":"a","zones":{"1":[0,0,1,1]}}]'); check("layouts: zones that are not a list", #layouts.user == 0)

check("settings: truncated", empty(settings.parse('{"enabled":false,')))
check("settings: array root", empty(settings.parse('[{"enabled":false}]')))
check("settings: nested name ignored, top-level value kept", settings.parse('{"unknown":{"enabled":false},"enabled":true}').enabled == true)
check("settings: escaped string decoded", settings.parse('{"bar_text_font":"Noto\\u0020Sans"}').bar_text_font == "Noto Sans")
check("settings: a string is not a boolean", settings.parse('{"enabled":"false"}').enabled == nil)

if failed > 0 then print(failed .. " failure(s)"); os.exit(1) end
print("userdata ok")
