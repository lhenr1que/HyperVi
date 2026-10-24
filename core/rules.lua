-- Per-app window rules (by window class), kept in ~/.config/kwinish/rules.json and applied when a window opens.
-- Pure; runs under plain lua. Shape: {"kitty":{"above":true,"workspace":3,"rect":[x,y,w,h]}, ...}
local M = {}

M.bools = { above = true, below = true, noframe = true, nocapture = true }

-- Classes become JSON keys read back as Lua keys, so only plain characters are allowed.
function M.valid_class(c) return type(c) == "string" and c ~= "" and #c <= 80 and c:match("^[%w%-%._]+$") ~= nil end

local function valid_rect(r)
  if type(r) ~= "table" or #r ~= 4 then return false end
  for i = 1, 4 do if type(r[i]) ~= "number" then return false end end
  return r[3] > 0 and r[4] > 0
end

-- Text of rules.json -> { class = { above = true, workspace = 3, rect = {x,y,w,h} } }; bad entries are dropped, bad text gives {}.
function M.parse(text)
  local out = {}
  if not text or text == "" then return out end
  -- JSON arrays become tables first, then the "key": pairs become ["key"]= (those brackets are Lua's, added last)
  local src = text:gsub("%[", "{"):gsub("%]", "}"):gsub('"([^"]*)"%s*:', function(k) return string.format("[%q]=", k) end)
  local chunk = load("return " .. src, "rules.json", "t", {})
  local ok, t = pcall(chunk or error)
  if not ok or type(t) ~= "table" then return out end
  for class, r in pairs(t) do
    if M.valid_class(class) and type(r) == "table" then
      local e = {}
      for k in pairs(M.bools) do if r[k] == true then e[k] = true end end
      if math.type(r.workspace) == "integer" and r.workspace > 0 then e.workspace = r.workspace end
      if valid_rect(r.rect) then e.rect = { r.rect[1], r.rect[2], r.rect[3], r.rect[4] } end
      if next(e) then out[class] = e end
    end
  end
  return out
end

local function sorted(t)
  local ks = {}
  for k in pairs(t) do ks[#ks + 1] = k end
  table.sort(ks)
  return ks
end

function M.serialize(rules)
  local parts = {}
  for _, class in ipairs(sorted(rules)) do
    local r, f = rules[class], {}
    for _, k in ipairs(sorted(M.bools)) do if r[k] then f[#f + 1] = string.format('"%s":true', k) end end
    if r.workspace then f[#f + 1] = string.format('"workspace":%d', r.workspace) end
    if r.rect then f[#f + 1] = string.format('"rect":[%d,%d,%d,%d]', r.rect[1], r.rect[2], r.rect[3], r.rect[4]) end
    parts[#parts + 1] = string.format('"%s":{%s}', class, table.concat(f, ","))
  end
  return "{" .. table.concat(parts, ",") .. "}"
end

-- Turn `key` on for `class` with `value` (true for the booleans, a number for workspace, a rect table), or off when it is
-- already on. Returns whether it is now on. Above and below exclude each other.
function M.toggle(rules, class, key, value)
  if not M.valid_class(class) then return false end
  local r = rules[class] or {}
  local on = r[key] == nil
  r[key] = on and value or nil
  if on and key == "above" then r.below = nil elseif on and key == "below" then r.above = nil end
  rules[class] = next(r) and r or nil
  return on
end

function M.forget(rules, class) rules[class] = nil end

-- shortcuts.json ({"kitty":"SUPER + ALT + K"}, written by `kwinish-ui shortcut`) -> { class = combo }. Only plain classes and
-- combos made of capital letters, digits, '+' and spaces survive, so nothing odd reaches hl.bind.
function M.parse_shortcuts(text)
  local out = {}
  for class, combo in (text or ""):gmatch('"([^"]*)"%s*:%s*"([^"]*)"') do
    if M.valid_class(class) and combo:match("^[%u%d%+ ]+$") and #combo <= 40 then out[class] = combo end
  end
  return out
end

return M
