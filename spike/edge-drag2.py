# usage: edge-drag2.py x y left|right|top : drag the bar grabbed at (x, y) to a screen edge and release
import subprocess, sys
x, y, edge = int(sys.argv[1]), int(sys.argv[2]), sys.argv[3]
end = {"left": [(x-60, y-30), (x-200, y-40), (x-400, y-40), (20, y-40), (1, y-40)],
       "right": [(x+100, y-30), (x+400, y-40), (1500, y-40), (1900, y-40), (1919, y-40)],
       "top": [(x+10, y-60), (x+60, y-150), (x+160, y-210), (x+300, 30), (x+330, 2)]}[edge]
pts = [f"{x},{y},0.4", f"{x},{y},0.2,d"] + [f"{px},{py},0.06" for px, py in end] + [f"{end[-1][0]},{end[-1][1]},0.4,u"]
subprocess.run(["python3", "-I", "../ryoku-vm/tools/pointer-path.py", "1920", "1080", *pts])
