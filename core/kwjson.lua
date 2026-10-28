-- JSON reader for the user's files. Pure Lua, no `load`: the files are read while Hyprland loads its config, and code in them could
-- hang the compositor. Decodes RFC 8259 JSON as plain data, refusing the whole document on any error; accepts and refuses what
-- serde_json (kwinish-ui) does, checked by core/testdata/json-cases.txt. Limits: 1 MiB of text, 32 levels of arrays/objects.
local M = {}

M.LIMIT, M.DEPTH = 1048576, 32

-- JSON null: a unique value that is not a table, so no validator mistakes it for an object or a list (and arrays keep their length).
M.null = function() end

local array_mt, object_mt = { __name = "json array" }, { __name = "json object" }
function M.is_array(v) return getmetatable(v) == array_mt end
function M.is_object(v) return getmetatable(v) == object_mt end

local escapes = { ['"'] = '"', ["\\"] = "\\", ["/"] = "/", b = "\b", f = "\f", n = "\n", r = "\r", t = "\t" }
local literals = { ["true"] = true, ["false"] = false, null = M.null }

-- text -> value, or nil and a message with the byte position.
function M.decode(text)
  if type(text) ~= "string" then return nil, "not text" end
  if #text > M.LIMIT then return nil, "larger than 1 MiB" end
  if not utf8.len(text) then return nil, "not valid UTF-8" end
  local pos = 1
  local function fail(msg) error({ msg = msg .. " at byte " .. pos }, 0) end
  local function skip() pos = text:find("[^ \t\n\r]", pos) or #text + 1 end

  local function str() -- pos is on the opening quote
    local out, i = {}, pos + 1
    while true do
      local j = text:find('["\\\0-\31]', i)
      if not j then fail("unterminated string") end
      out[#out + 1] = text:sub(i, j - 1)
      local c = text:sub(j, j)
      if c == '"' then pos = j + 1; return table.concat(out) end
      pos = j
      if c ~= "\\" then fail("control character in a string") end
      local e = text:sub(j + 1, j + 1)
      if escapes[e] then out[#out + 1] = escapes[e]; i = j + 2
      elseif e == "u" then
        local cp = tonumber(text:match("^%x%x%x%x", j + 2) or "", 16)
        if not cp then fail("bad \\u escape") end
        i = j + 6
        if cp >= 0xD800 and cp <= 0xDBFF then -- a high surrogate needs its low half next
          local lo = tonumber(text:match("^\\u(%x%x%x%x)", i) or "", 16)
          if not (lo and lo >= 0xDC00 and lo <= 0xDFFF) then fail("lone surrogate") end
          cp, i = 0x10000 + (cp - 0xD800) * 0x400 + (lo - 0xDC00), i + 6
        elseif cp >= 0xDC00 and cp <= 0xDFFF then fail("lone surrogate") end
        out[#out + 1] = utf8.char(cp)
      else fail("bad escape") end
    end
  end

  local function num()
    local i = pos
    if text:sub(i, i) == "-" then i = i + 1 end
    local int = text:match("^%d+", i)
    if not int or (#int > 1 and int:sub(1, 1) == "0") then fail("bad number") end
    i = i + #int
    local frac = text:match("^%.(%d*)", i)
    if frac then
      if frac == "" then fail("bad number") end
      i = i + 1 + #frac
    end
    local exp = text:match("^[eE][-+]?%d*", i)
    if exp then
      if not exp:match("%d$") then fail("bad number") end
      i = i + #exp
    end
    local n = tonumber(text:sub(pos, i - 1)) -- an integer when it is one and fits, else a float
    if n ~= n or n == math.huge or n == -math.huge then fail("number out of range") end
    pos = i
    return n
  end

  local value
  local function container(close, mt, depth, member)
    if depth >= M.DEPTH then fail("deeper than " .. M.DEPTH .. " levels") end
    local t, n = setmetatable({}, mt), 0
    pos = pos + 1; skip()
    if text:sub(pos, pos) == close then pos = pos + 1; return t end
    while true do
      skip()
      if member then
        if text:sub(pos, pos) ~= '"' then fail("expected a member name") end
        local k = str(); skip()
        if text:sub(pos, pos) ~= ":" then fail("expected ':'") end
        pos = pos + 1
        t[k] = value(depth + 1)
      else
        n = n + 1
        t[n] = value(depth + 1)
      end
      skip()
      local c = text:sub(pos, pos)
      if c == close then pos = pos + 1; return t end
      if c ~= "," then fail("expected ',' or '" .. close .. "'") end
      pos = pos + 1
    end
  end

  function value(depth)
    skip()
    local c = text:sub(pos, pos)
    if c == "{" then return container("}", object_mt, depth, true) end
    if c == "[" then return container("]", array_mt, depth, false) end
    if c == '"' then return str() end
    if c == "-" or c:match("%d") then return num() end
    for word, v in pairs(literals) do
      if text:sub(pos, pos + #word - 1) == word then pos = pos + #word; return v end
    end
    fail("expected a value")
  end

  local ok, v = pcall(function()
    local v = value(0)
    skip()
    if pos <= #text then fail("unexpected text after the value") end
    return v
  end)
  if ok then return v end
  return nil, type(v) == "table" and v.msg or tostring(v)
end

-- The text of `path`, at most LIMIT + 1 bytes (enough for decode to refuse a larger file without reading it all); nil when absent.
function M.read(path)
  local f = io.open(path, "rb")
  if not f then return nil end
  local text = f:read(M.LIMIT + 1)
  f:close()
  return text
end

return M
