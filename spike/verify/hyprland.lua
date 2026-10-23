hl.monitor({ output = "", mode = "preferred", position = "auto", scale = 1 })
dofile("/home/ryo/Downloads/ryoku-work/kwinish/recipes/native.lua")
os.getenv = (function(f) return function(k) if k == "HOME" then return "/home/ryo/Downloads/ryoku-work/kwinish/spike/verify" end return f(k) end end)(os.getenv)
dofile("/home/ryo/Downloads/ryoku-work/kwinish/recipes/hyprland-snippet.lua")
