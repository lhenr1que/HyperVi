-- Real-bind test: loads kwinish from this checkout and binds only keys that are unlikely to clash.
-- Use: add  dofile("/home/ryo/Downloads/ryoku-work/kwinish/spike/verify/live-test.lua")  to user.lua, reload, try, then remove the line and reload.
package.path = "/home/ryo/Downloads/ryoku-work/kwinish/core/?.lua;" .. package.path
local kw = require("kwinish")
kw.setup({  })
local sw = require("switcher")
sw.setup()
-- Clash-free against the owner's Ryoku binds (checked with hyprctl binds on 2026-10-24): SUPER+CTRL+..., ALT+grave for the switcher.
local off = {}
for _, d in ipairs({ "Left", "Right", "Up", "Down" }) do off["SUPER + CTRL + ALT + " .. d] = false end
require("binds").install(kw, { mod = "SUPER + CTRL", switcher = sw, switcher_keys = { "ALT + grave", "ALT + SHIFT + grave" }, menu = "ALT + F3", override = off })
