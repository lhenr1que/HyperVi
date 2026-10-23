-- Recipes for KWin behaviours Hyprland already has. Paste what you want into hyprland.lua.
-- Each block names the KWin feature it covers (numbers refer to docs/referencia/kwin-vs-hyprland-marcado.md).

-- 8: snap to window and screen edges while dragging
hl.config({ general = { snap = { enabled = true, window_gap = 10, monitor_gap = 10, respect_gaps = true } } })

-- 29: dim inactive windows
hl.config({ decoration = { dim_inactive = true, dim_strength = 0.15 } })

-- 13: "application is not responding" dialog
hl.config({ misc = { enable_anr_dialog = true } })

-- 10: focus follows the pointer; clicking still raises (kwinish autoraise_ms adds raise-on-hover)
hl.config({ input = { follow_mouse = 1 } })
