# usage: edge-drag.py left|right|top : drag the bar of the floating test window (bar centre 700,386) to a screen edge and release
import subprocess, sys
paths = {
  "left":  "640,300,0.05 500,250,0.05 300,200,0.05 100,200,0.05 20,200,0.05 1,200,0.3 1,200,0.3,u",
  "right": "800,300,0.05 1000,250,0.05 1400,200,0.05 1800,200,0.05 1900,200,0.05 1919,200,0.3 1919,200,0.3,u",
  "top":   "640,300,0.05 700,200,0.05 800,100,0.05 900,40,0.05 920,10,0.05 930,2,0.4 930,2,0.3,u",
}
subprocess.run(["python3", "-I", "../ryoku-vm/tools/pointer-path.py", "1920", "1080", "700,386,0.4", "700,386,0.2,d", *paths[sys.argv[1]].split()])
