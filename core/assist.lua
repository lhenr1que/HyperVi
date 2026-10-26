-- Snap Assist logic: after a window is put in a layout zone, which zone is still empty and which windows could fill it.
-- A window counts as in a zone when it sits on that zone's rect (within a few px), as a snap leaves it. Pure; runs under plain lua. `wins` are {address, class, title, x, y, w, h} (floating windows of the same workspace and monitor,
-- the one just placed excluded), `rects` the zone rects, `placed` the index of the zone just filled.
local M = {}

function M.plan(rects, wins, placed)
  local taken, cands = { [placed] = true }, {}
  for _, w in ipairs(wins) do
    local inside = false
    for i, r in ipairs(rects) do
      if math.abs(w.x - r.x) <= 4 and math.abs(w.y - r.y) <= 4 and math.abs(w.w - r.w) <= 4 and math.abs(w.h - r.h) <= 4 then
        taken[i], inside = true, true; break
      end
    end
    if not inside then cands[#cands + 1] = w end
  end
  if #cands == 0 then return nil end
  for i, r in ipairs(rects) do
    if not taken[i] then return { zone = i, rect = r, candidates = cands } end
  end
end

local function esc(s)
  return (tostring(s or ""):gsub('[%c"\\]', function(c) return string.format("\\u%04x", c:byte()) end))
end

-- The JSON kwinish-ui assist reads: {"monitor":..,"zone":N,"rect":[x,y,w,h],"items":[{"address","class","title"}]}
function M.json(monitor, plan)
  local items = {}
  for _, w in ipairs(plan.candidates) do
    items[#items + 1] = string.format('{"address":"%s","class":"%s","title":"%s"}', esc(w.address), esc(w.class), esc(w.title))
  end
  local r = plan.rect
  return string.format('{"monitor":"%s","zone":%d,"rect":[%d,%d,%d,%d],"items":[%s]}', esc(monitor), plan.zone,
    math.floor(r.x + 0.5), math.floor(r.y + 0.5), math.floor(r.w + 0.5), math.floor(r.h + 0.5), table.concat(items, ","))
end

return M
