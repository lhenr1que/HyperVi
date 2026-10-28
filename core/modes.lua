-- Workspace modes: tiling (Hyprland's dwindle), scrolling (its scrolling layout) or floating (every window floats). Pure; runs
-- under plain lua. The floating mode works on the windows themselves, as Hyprland has no floating layout (idea and numbers as in
-- Omarchy's workspace-modes.lua, MIT; this is an independent implementation).
local kwjson = require("kwjson")
local M = {}

M.valid = { tiling = true, scrolling = true, floating = true }

-- The Hyprland tiled layout under each mode; floating keeps dwindle under its floated windows, so leaving the mode tiles them again.
function M.engine(mode) return mode == "scrolling" and "scrolling" or "dwindle" end

-- Mode a Hyprland tiled layout stands for, when the workspace has no mode of its own.
function M.from_engine(layout) return layout == "scrolling" and "scrolling" or "tiling" end

function M.next(mode, back)
  local order = back and { tiling = "floating", floating = "scrolling", scrolling = "tiling" } or { tiling = "scrolling", scrolling = "floating", floating = "tiling" }
  return order[mode or "tiling"] or "tiling"
end

-- {"3":"floating","4":"scrolling"} -> { ["3"] = "floating", ["4"] = "scrolling" }; odd entries are dropped, a refused text gives {}
-- and the reason (kwjson: data only).
function M.parse(text)
  local out = {}
  if not text or text == "" then return out end
  local t, err = kwjson.decode(text)
  if not kwjson.is_object(t) then return out, err or "not a JSON object" end
  for ws, mode in pairs(t) do
    if ws:match("^%d+$") and tonumber(ws) >= 1 and M.valid[mode] then out[ws] = mode end
  end
  return out
end

function M.serialize(modes)
  local ks = {}
  for k in pairs(modes) do ks[#ks + 1] = k end
  table.sort(ks, function(a, b) return tonumber(a) < tonumber(b) end)
  local parts = {}
  for _, k in ipairs(ks) do parts[#parts + 1] = string.format('"%s":"%s"', k, modes[k]) end
  return "{" .. table.concat(parts, ",") .. "}"
end

-- Where a window floated by the mode opens inside `area` {x, y, w, h}: 64% x 68% (at most 1100 x 760), centred, and stepped
-- 24 px per `step` (0..4) so windows floated together do not sit exactly on one another.
function M.open_rect(area, step)
  local w, h = math.min(1100, math.floor(area.w * 0.64)), math.min(760, math.floor(area.h * 0.68))
  local off = (step % 5) * 24 - 48
  return { x = math.floor(area.x + (area.w - w) / 2) + off, y = math.floor(area.y + (area.h - h) / 2) + off, w = w, h = h }
end

-- workspace-columns.json: {"2":3} = workspace 2 keeps 3 columns per screen (1 to 3). Absent = left alone. Read like M.parse.
function M.parse_columns(text)
  local out = {}
  if not text or text == "" then return out end
  local t, err = kwjson.decode(text)
  if not kwjson.is_object(t) then return out, err or "not a JSON object" end
  for ws, n in pairs(t) do
    if ws:match("^%d+$") and tonumber(ws) >= 1 and math.type(n) == "integer" and n >= 1 and n <= 3 then out[ws] = n end
  end
  return out
end

function M.serialize_columns(cols)
  local ks = {}
  for k in pairs(cols) do ks[#ks + 1] = k end
  table.sort(ks, function(a, b) return tonumber(a) < tonumber(b) end)
  local parts = {}
  for _, k in ipairs(ks) do parts[#parts + 1] = string.format('"%s":%d', k, cols[k]) end
  return "{" .. table.concat(parts, ",") .. "}"
end

return M
