package.path = (arg[0]:match("(.*/)") or "./") .. "?.lua;" .. package.path
local binds = require("binds")
local seen, calls = {}, {}
hl = { bind = function(keys, fn) seen[#seen + 1] = keys; calls[keys] = fn end }
local kw = setmetatable({}, { __index = function(_, name) return function(a) calls[#calls + 1] = name .. ":" .. tostring(a) end end })
binds.install(kw, { override = { ["SUPER + D"] = false, ["SUPER + ALT + T"] = "SUPER + SHIFT + T" } })
local set = {}
for _, k in ipairs(seen) do assert(not set[k], "duplicate " .. k); set[k] = true end
assert(#seen == #binds.rows - 1, "dropped one, got " .. #seen)
assert(not set["SUPER + D"] and set["SUPER + SHIFT + T"] and not set["SUPER + ALT + T"], "override")
seen = {}; calls = {}; hl.bind = function(k, f) seen[#seen + 1] = k; calls[k] = f end
binds.install(kw)
calls["SUPER + ALT + Left"](); assert(calls[#calls] == "tile:left", "dispatch")
print("binds ok")
