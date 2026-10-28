-- The JSON reader of the core (core/kwjson.lua): data only, limits, and the same verdicts as kwinish-ui (core/testdata/json-cases.txt).
local dir = arg[0]:match("(.*/)") or "./"
package.path = dir .. "?.lua;" .. package.path
local J = require("kwjson")

local function bad(text, msg)
  local v, err = J.decode(text)
  assert(v == nil and type(err) == "string", (msg or "should be refused") .. ": " .. tostring(text):sub(1, 60))
end

-- shared verdicts
local n = 0
for line in io.lines(dir .. "testdata/json-cases.txt") do
  local verdict, text = line:match("^(%a+) ?(.*)$")
  if verdict == "ok" then
    local v, err = J.decode(text); assert(v ~= nil, "should decode: " .. text .. " (" .. tostring(err) .. ")"); n = n + 1
  elseif verdict == "bad" then bad(text); n = n + 1 end
end
assert(n > 50, "cases file read")

-- null, arrays and objects stay apart, empty ones included
assert(J.decode("null") == J.null and type(J.null) ~= "table", "null is a sentinel no validator takes for a table")
local a, o = J.decode("[]"), J.decode("{}")
assert(J.is_array(a) and #a == 0 and not J.is_object(a), "empty array")
assert(J.is_object(o) and next(o) == nil and not J.is_array(o), "empty object")
local mix = J.decode('[null,1,"x",true,false,{"k":null}]')
assert(#mix == 6 and mix[1] == J.null and mix[2] == 1 and mix[3] == "x" and mix[4] == true and mix[5] == false, "array keeps every slot")
assert(J.is_object(mix[6]) and mix[6].k == J.null, "null member kept")
assert(J.decode('{"a":1,"a":2}').a == 2, "duplicate member: the last one wins (as serde_json)")
assert(J.decode('{"__index":1}').__index == 1 and J.decode('{"__index":1}').x == nil, "keys are plain data")

-- strings
assert(J.decode([["a\"b\\c\/d\b\f\n\r\t"]]) == 'a"b\\c/d\b\f\n\r\t', "simple escapes")
assert(J.decode([["\u00e9\u20ac"]]) == "é€" and J.decode([["\ud83d\ude00"]]) == "😀", "\\u escapes and surrogate pairs")
assert(J.decode([["\ud83d\ude00"]]) ~= nil and #J.decode([["\ud83d\ude00"]]) == 4, "a pair is one 4-byte character")
assert(J.decode('"é"') == "é" and J.decode('"a\127b"') == "a\127b", "raw UTF-8 and DEL pass")
bad('"a\nb"', "raw control character"); bad('"\0"', "raw NUL")
bad('"\255"', "invalid UTF-8"); bad('"\192\175"', "overlong UTF-8"); bad('"\237\160\128"', "UTF-8 surrogate")
assert(J.decode('{"k":"v"}').k == "v")

-- numbers: integers stay integers, the rest are floats, never infinite
assert(math.type(J.decode("12")) == "integer" and J.decode("-12") == -12)
local nz = J.decode("-0")
assert(math.type(nz) == "float" and nz == 0 and 1 / nz < 0, "-0 is the float -0.0, as serde_json reads it")
assert(math.type(J.decode("1.0")) == "float" and J.decode("1e2") == 100.0 and math.type(J.decode("1e2")) == "float")
assert(J.decode("-0.5e+3") == -500.0 and J.decode("1E-2") == 0.01)
assert(math.type(J.decode("9223372036854775808")) == "float", "too big for an integer: float, like serde_json's f64")
bad("1e999", "infinite"); bad("[-1e999]", "infinite")

-- structure
bad("[1] x", "trailing garbage"); bad("[1]]"); bad("{}{}"); bad(" ", "nothing")
assert(J.decode(" \t\n\r[1] \n\t")[1] == 1, "JSON whitespace around")
bad("\239\187\191[]", "byte order mark is not whitespace")
bad(nil, "not a string"); bad(42, "not a string")
assert(J.decode('{"k":(function() while true do end end)()}') == nil, "Lua in the file is refused, never run")

-- depth: 32 levels pass, 33 do not
assert(J.decode(string.rep("[", 32) .. string.rep("]", 32)), "32 levels")
bad(string.rep("[", 33) .. string.rep("]", 33), "33 levels")
assert(J.decode(string.rep('{"a":', 31) .. "[]" .. string.rep("}", 31)), "32 levels, objects")
bad(string.rep('{"a":', 32) .. "[]" .. string.rep("}", 32), "33 levels, objects")

-- size: 1 MiB passes, one byte more does not
assert(J.LIMIT == 1048576 and J.DEPTH == 32)
local big = '"' .. string.rep("a", J.LIMIT - 2) .. '"'
assert(#big == J.LIMIT and #J.decode(big) == J.LIMIT - 2, "exactly 1 MiB")
bad('"' .. string.rep("a", J.LIMIT - 1) .. '"', "1 MiB + 1")

-- read: at most LIMIT + 1 bytes, nothing for a missing file
local file = os.tmpname()
local f = assert(io.open(file, "wb")); f:write(string.rep("x", 2 * J.LIMIT)); f:close()
assert(#J.read(file) == J.LIMIT + 1, "read stops after LIMIT + 1 bytes")
f = assert(io.open(file, "wb")); f:write("{}"); f:close()
assert(J.read(file) == "{}")
os.remove(file)
assert(J.read(file) == nil, "missing file")

-- write: a temporary file in the same directory and a rename. Tested on a file system in memory where one operation fails.
local function fakefs(files, fail)
  local fs = {}
  function fs.mkdir() return true end
  function fs.open(path, mode)
    if fail == "open:" .. path then return nil, path .. ": open failed" end
    if mode:match("r") then
      if files[path] == nil then return nil, path .. ": absent" end
      return { read = function() if fail == "read:" .. path then return nil, "read failed" end return files[path] end, close = function() return true end }
    end
    files[path] = "" -- "w" truncates, as io.open does
    return {
      write = function(self, s) if fail == "write:" .. path then return nil, "disk full" end files[path] = files[path] .. s; return self end,
      close = function() if fail == "close:" .. path then return nil, "close failed" end return true end,
    }
  end
  function fs.rename(a, b) if fail == "rename:" .. b then return nil, "rename failed" end files[b], files[a] = files[a], nil; return true end
  function fs.remove(p) files[p] = nil; return true end
  return fs
end
local P = "/cfg/kwinish/rules.json"
local function files_after(fail, old, check, text)
  local files = { [P] = old }
  local ok, err = J.write(P, text == nil and "{}" or text, check, fakefs(files, fail))
  return files, ok, err
end
local fs_ok, ok, err = files_after(nil, "old")
assert(ok == true and fs_ok[P] == "{}" and fs_ok[P .. ".tmp"] == nil, "written through a temporary file, none left")
for _, fail in ipairs({ "open:" .. P .. ".tmp", "write:" .. P .. ".tmp", "close:" .. P .. ".tmp", "rename:" .. P }) do
  local files, ok2, err2 = files_after(fail, "old bytes")
  assert(ok2 == nil and type(err2) == "string", fail .. ": failure reported")
  assert(files[P] == "old bytes" and files[P .. ".tmp"] == nil, fail .. ": old file intact, temporary removed")
  files = files_after(fail, nil)
  assert(files[P] == nil and files[P .. ".tmp"] == nil, fail .. ": first write leaves no file")
end
local files = files_after(nil, "old", nil, false)
assert(files[P] == "old", "nothing to write (serialization failed): nothing touched")
-- check: an existing file the reader refuses is kept byte for byte before being replaced, and is never lost
local function object(t) return J.is_object((J.decode(t))) end
files = files_after(nil, '{"kitty":{"above":true}', object)
assert(files[P] == "{}" and files[P .. ".bak"] == '{"kitty":{"above":true}', "refused file copied to .bak, then replaced")
files = files_after(nil, '{"kitty":{}}', object)
assert(files[P] == "{}" and files[P .. ".bak"] == nil, "a valid file needs no copy")
files = files_after(nil, " \n", object)
assert(files[P] == "{}" and files[P .. ".bak"] == nil, "an empty file needs no copy")
for _, fail in ipairs({ "write:" .. P .. ".bak.tmp", "rename:" .. P .. ".bak", "read:" .. P }) do
  files, ok, err = files_after(fail, "not json", object)
  assert(ok == nil and err and files[P] == "not json" and files[P .. ".tmp"] == nil, fail .. ": no copy, no replacement")
end
assert(J.quote("/a b/it's") == [['/a b/it'\''s']], "one shell word")
assert(io.popen("printf %s " .. J.quote("it's $HOME `x`")):read("a") == "it's $HOME `x`", "the shell gets the text back unchanged")
-- the real thing, in a new directory (created by write)
local base = os.tmpname(); os.remove(base)
local real = base .. "/sub/x.json"
assert(J.write(real, '{"a":1}') == true and J.read(real) == '{"a":1}' and io.open(real .. ".tmp") == nil, "real write")
os.remove(real); os.remove(base .. "/sub"); os.remove(base)

print("kwjson ok")
