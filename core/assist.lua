-- Snap Assist logic: after a window is put in a layout zone, which zone is still empty and which windows could fill it.
-- A window counts as in a zone when it sits on that zone's rect (within a few px), as a snap leaves it. Pure; runs under plain lua. `wins` are {address, class, title, x, y, w, h} (floating windows of the same workspace and monitor,
-- the one just placed excluded), `rects` the zone rects (false for a zone with no rect, see layouts.zone_rects), `placed` the
-- index of the zone just filled.
local M = {}

function M.plan(rects, wins, placed)
  local taken, cands = { [placed] = true }, {}
  for _, w in ipairs(wins) do
    local inside = false
    for i, r in ipairs(rects) do
      if r and math.abs(w.x - r.x) <= 4 and math.abs(w.y - r.y) <= 4 and math.abs(w.w - r.w) <= 4 and math.abs(w.h - r.h) <= 4 then
        taken[i], inside = true, true; break
      end
    end
    if not inside then cands[#cands + 1] = w end
  end
  if #cands == 0 then return nil end
  for i, r in ipairs(rects) do
    if r and not taken[i] then return { zone = i, rect = r, candidates = cands } end
  end
end

local function esc(s)
  return (tostring(s or ""):gsub('[%c"\\]', function(c) return string.format("\\u%04x", c:byte()) end))
end

-- The JSON kwinish-ui assist reads: {"generation":N,"monitor":..,"zone":N,"rect":[x,y,w,h],"items":[{"address","class","title"}]}
function M.json(monitor, plan, generation)
  local items = {}
  for _, w in ipairs(plan.candidates) do
    items[#items + 1] = string.format('{"address":"%s","class":"%s","title":"%s"}', esc(w.address), esc(w.class), esc(w.title))
  end
  local r = plan.rect
  return string.format('{"generation":%d,"monitor":"%s","zone":%d,"rect":[%d,%d,%d,%d],"items":[%s]}', generation, esc(monitor), plan.zone,
    math.floor(r.x + 0.5), math.floor(r.y + 0.5), math.floor(r.w + 0.5), math.floor(r.h + 0.5), table.concat(items, ","))
end

-- Preserve the last published generation across core reloads; never expose partial JSON.
function M.write(path, monitor, plan)
  local old = io.open(path, "r")
  local generation = 0
  if old then generation = tonumber(old:read("*a"):match('"generation":(%d+)')) or 0; old:close() end
  generation = generation + 1
  local text = M.json(monitor, plan, generation)
  local tmp = path .. ".tmp"
  local f = io.open(tmp, "w")
  if not f then return nil end
  local ok = f:write(text)
  local closed = f:close()
  if ok and closed and os.rename(tmp, path) then return generation end
  os.remove(tmp)
end

return M
