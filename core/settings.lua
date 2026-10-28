-- User settings, kept in ~/.config/kwinish/settings.json (written by `kwinish-ui settings`) and applied each time the core loads.
-- Pure; runs under plain lua. Shape: {"bar_height":28,"bar_color":"rgba(26262ef7)","snap_assist":false, ...}
-- Every key except snap_assist is a plugin:kwinish:* option; ui/src/settings.rs has the same list (keep them in step).
local kwjson = require("kwjson")
local M = {}

local enum = function(...) local t = {}; for _, v in ipairs({ ... }) do t[v] = true end; return t end
M.spec = {
  enabled = "bool", floating_only = "bool", bar_part_of_window = "bool", bar_precedence_over_border = "bool",
  bar_title_enabled = "bool", bar_blur = "bool", default_buttons = "bool", icon_on_hover = "bool", show_icon = "bool",
  snap_top_edge = "bool", snap_preview = "bool", snap_assist = "bool",
  bar_height = { 10, 80 }, bar_padding = { 0, 40 }, bar_text_size = { 6, 40 }, bar_text_weight = { 100, 1000 },
  bar_button_padding = { 0, 40 }, seam_resistance = { 0, 2000 },
  bar_color = "color", inactive_bar_color = "color", ["col.text"] = "color", ["inactive_col.text"] = "color",
  button_hover_color = "color", pinned_color = "color", preview_color = "color", inactive_button_color = "color",
  bar_text_font = "font",
  bar_text_align = enum("left", "center"), bar_buttons_alignment = enum("left", "right"),
  on_double_click = enum("maximize", "fullscreen", "minimize", "float", "close", "menu", "none"),
}
M.core_keys = { snap_assist = true } -- read by the core itself, not passed to the plugin

local function valid(kind, v)
  if kind == "bool" then return type(v) == "boolean" end
  if kind == "color" then return type(v) == "string" and v:match("^rgba%(%x%x%x%x%x%x%x%x%)$") ~= nil end
  if kind == "font" then return type(v) == "string" and #v <= 60 and v:match("^[%w][%w %-_]*$") ~= nil end
  if kind[1] then return math.type(v) == "integer" and v >= kind[1] and v <= kind[2] end
  return type(v) == "string" and kind[v] == true
end

-- Text of settings.json -> { key = value } with unknown keys and bad values dropped, so one odd entry never costs the others. Read as
-- data (kwjson, no `load`: the file is read during config load); only top-level members count. A refused text gives {} and the reason.
function M.parse(text)
  local out = {}
  if type(text) ~= "string" or text == "" then return out end
  local t, err = kwjson.decode(text)
  if not kwjson.is_object(t) then return out, err or "not a JSON object" end
  for k, kind in pairs(M.spec) do
    if valid(kind, t[k]) then out[k] = t[k] end
  end
  return out
end

-- The plugin's share as one hl.config table per key, so one value the compositor refuses does not drop the rest.
function M.plugin_configs(s)
  local out = {}
  for k, v in pairs(s) do
    if not M.core_keys[k] then out[#out + 1] = { plugin = { kwinish = { [k] = v } } } end
  end
  table.sort(out, function(a, b) return next(a.plugin.kwinish) < next(b.plugin.kwinish) end)
  return out
end

return M
