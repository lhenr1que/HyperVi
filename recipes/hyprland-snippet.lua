-- Minimal setup. Put the kwinish/core directory on the Lua path, then add this to hyprland.lua.
package.path = os.getenv("HOME") .. "/.local/share/kwinish/core/?.lua;" .. package.path
local kw = require("kwinish")
kw.setup({ placement = "smart", autoraise_ms = 400 }) -- drop either option to turn it off
local sw = require("switcher")
sw.setup()                                             -- Alt+Tab; start the overlay with: exec-once = kwinish-ui
require("binds").install(kw, { mod = "SUPER", switcher = sw, menu = "ALT + F3" })        -- pass override = { ["SUPER + D"] = false } to drop a bind
