-- Default bind table for kwinish. `install` registers it with hl.bind; `list` is for printing.
-- Override or drop entries with opts.override = { ["SUPER + D"] = false, ["SUPER + ALT + T"] = "SUPER + SHIFT + T" }.
local M = {}

-- {keys (relative to mod), action name, argument}
M.rows = {
  { "ALT + Left",  "tile", "left" },        { "ALT + Right", "tile", "right" },
  { "ALT + Up",    "tile", "top" },         { "ALT + Down",  "tile", "bottom" },
  { "ALT + Q",     "tile", "topleft" },     { "ALT + W",     "tile", "topright" },
  { "ALT + A",     "tile", "bottomleft" },  { "ALT + S",     "tile", "bottomright" },
  { "ALT + Return", "tile", "max" },
  { "ALT + H",     "maximize_axis", "h" },  { "ALT + V",     "maximize_axis", "v" },
  { "CTRL + ALT + Left",  "pack", "left" }, { "CTRL + ALT + Right", "pack", "right" },
  { "CTRL + ALT + Up",    "pack", "up" },   { "CTRL + ALT + Down",  "pack", "down" },
  { "D",           "show_desktop" },
  { "ALT + T",     "keep_above" },
}

function M.list(mod)
  mod = mod or "SUPER"
  local out = {}
  for _, r in ipairs(M.rows) do out[#out + 1] = { keys = mod .. " + " .. r[1], action = r[2], arg = r[3] } end
  return out
end

function M.install(kw, opts)
  opts = opts or {}
  for _, b in ipairs(M.list(opts.mod)) do
    local keys = b.keys
    if opts.override and opts.override[keys] ~= nil then keys = opts.override[keys] end
    if keys then
      local fn, arg = kw[b.action], b.arg
      hl.bind(keys, function() fn(arg) end)
    end
  end
end

return M
