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
assert(J.decode([["é€"]]) == "é€" and J.decode([["😀"]]) == "😀", "\\u escapes and surrogate pairs")
assert(J.decode('"é"') == "é" and J.decode('"a\127b"') == "a\127b", "raw UTF-8 and DEL pass")
bad('"a\nb"', "raw control character"); bad('"\0"', "raw NUL")
bad('"\255"', "invalid UTF-8"); bad('"\192\175"', "overlong UTF-8"); bad('"\237\160\128"', "UTF-8 surrogate")
assert(J.decode('{"k":"v"}').k == "v")

-- numbers: integers stay integers, the rest are floats, never infinite
assert(math.type(J.decode("12")) == "integer" and J.decode("-12") == -12 and math.type(J.decode("-0")) == "integer")
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

print("kwjson ok")
