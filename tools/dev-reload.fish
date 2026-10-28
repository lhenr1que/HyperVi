#!/usr/bin/env fish
# Dev loop: load this checkout's plugin and Lua core into the running Hyprland.
# Reloading the plugin brings back the installed core (/usr/share/kwinish), which ignores the plugin's "@monitor" actions
# and snaps nothing, so both must be reloaded together.
set root (path resolve (status dirname)/..)
hyprctl plugin unload $root/plugin/kwinish-bars.so
hyprctl plugin load $root/plugin/kwinish-bars.so
hyprctl eval "package.path='$root/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
