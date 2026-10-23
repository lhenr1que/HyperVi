# 60-step drag at ~100 Hz over the spike title bar (bar centre 800,486 -> 620,366); result is in /tmp/claude-1000/bar-spike.log
import subprocess
pts = [f"{800-i*3},{486-i*2},0.01" for i in range(1, 61)]
subprocess.run(["python3", "-I", "../ryoku-vm/tools/pointer-path.py", "1920", "1080", "800,486,0.4", "800,486,0.3,d", *pts, "620,366,0.2", "620,366,0.3,u"])
