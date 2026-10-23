-- spike: runtime-only listener (hyprctl eval), logs event payloads; gone on reload
local LOG = "/home/ryo/Downloads/ryoku-work/kwinish/spike/events.log"
local function dump(v, d)
  d = d or 0
  if type(v) ~= "table" or d > 1 then return tostring(v) end
  local t = {}
  for k, x in pairs(v) do t[#t + 1] = tostring(k) .. "=" .. dump(x, d + 1) end
  table.sort(t)
  return "{" .. table.concat(t, ",") .. "}"
end
local function log(name, ...)
  local f = io.open(LOG, "a")
  if not f then return end
  f:write(string.format("%.3f %s %s\n", os.clock(), name, dump({ ... })))
  f:close()
end
KW_SPIKE = KW_SPIKE or {}
for _, n in ipairs({ "input.keyboard.key", "window.active", "window.open", "window.move_to_workspace", "window.update_rules" }) do
  if KW_SPIKE[n] then pcall(function() KW_SPIKE[n]:remove() end) end
  KW_SPIKE[n] = hl.on(n, function(...) log(n, ...) end)
end
