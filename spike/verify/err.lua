hl.monitor({ output = "", mode = "preferred", position = "auto", scale = 1 })
local ok, err = pcall(dofile, "/home/ryo/Downloads/ryoku-work/kwinish/spike/verify/live-test.lua")
if not ok then error("LIVE-TEST FAILED: " .. tostring(err)) end
