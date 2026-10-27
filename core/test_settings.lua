package.path = (arg[0]:match("(.*/)") or "./") .. "?.lua;" .. package.path
local S = require("settings")
local s = S.parse('{\n "bar_height": 32, "snap_assist": false, "bar_color": "rgba(26262ef7)", "col.text": "rgba(F0F0F6FF)",\n'
  .. ' "bar_text_font": "JetBrains Mono", "on_double_click": "close", "seam_resistance": 0 }')
assert(s.bar_height == 32 and s.snap_assist == false and s.bar_color == "rgba(26262ef7)" and s["col.text"] == "rgba(F0F0F6FF)")
assert(s.bar_text_font == "JetBrains Mono" and s.on_double_click == "close" and s.seam_resistance == 0)
local bad = S.parse('{"bar_height": 500, "bar_padding": 2.5, "enabled": "yes", "bar_color": "red", "bar_text_align": "middle",'
  .. ' "bar_text_font": "x\'); os.exit(", "unknown": 1, "preview_color": "rgba(12345678) "}')
assert(next(bad) == nil, "out-of-range, wrong-type, unknown and odd values dropped")
assert(next(S.parse("garbage {{")) == nil and next(S.parse(nil)) == nil and next(S.parse('{"k":os.exit()}')) == nil)
local cfg = S.plugin_configs({ snap_assist = false, bar_height = 30, enabled = true })
assert(#cfg == 2 and cfg[1].plugin.kwinish.bar_height == 30 and cfg[2].plugin.kwinish.enabled == true, "core keys stay out, one table per key")
local mixed = S.parse('{"bar_height":28,"unknown":[1,2],"bar_text_font":"A\\u0020B","enabled":(function() while true do end end)(),"show_icon":false}')
assert(mixed.bar_height == 28 and mixed.show_icon == false and mixed.bar_text_font == nil and mixed.enabled == nil, "odd entries cost only themselves, no code runs")
local sub = S.parse('{"inactive_col.text":"rgba(11223344)"}')
assert(sub["inactive_col.text"] == "rgba(11223344)" and sub["col.text"] == nil, "a key is not matched inside a longer one")
assert(next(S.parse('{"bar_height":"34","enabled":"true"}')) == nil, "quoted numbers and booleans are strings")
print("settings ok")
