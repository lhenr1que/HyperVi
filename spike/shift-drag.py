# shift-drag.py X Y HOLD: hold Shift, press the bar of the test window at (700,386), drag to X,Y, hold HOLD seconds, release the button, then Shift
import os, fcntl, struct, sys, time
import json, subprocess
# an absolute virtual pointer spans the bounding box of all monitors, not just the first one
_m = json.loads(subprocess.run(["hyprctl", "monitors", "-j"], capture_output=True, text=True).stdout)
_b = [(m["x"], m["y"], m["x"] + m["width"] / m["scale"], m["y"] + m["height"] / m["scale"]) for m in _m]
OX, OY = min(b[0] for b in _b), min(b[1] for b in _b)
W, H = max(b[2] for b in _b) - OX, max(b[3] for b in _b) - OY
tx, ty, hold = int(sys.argv[1]), int(sys.argv[2]), float(sys.argv[3])
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
fcntl.ioctl(fd, 0x40045564, 1)
for k in list(range(1, 249)) + [0x110]: fcntl.ioctl(fd, 0x40045565, k)  # keys (a full keyboard, so udev tags it as one) and the left button
fcntl.ioctl(fd, 0x40045564, 3)                                      # absolute axes
fcntl.ioctl(fd, 0x40045567, 0); fcntl.ioctl(fd, 0x40045567, 1)
am = [0] * 64; am[0] = am[1] = 32767
os.write(fd, struct.pack("80sHHHHi", b"kwinish-shift-dev", 3, 0x1234, 0x5685, 1, 0) + struct.pack("64i", *am) + b"\0" * (4 * 64 * 3))
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
def go(x, y, h=0.06): s(3, 0, round((x - OX) * 32767 / (W - 1))); s(3, 1, round((y - OY) * 32767 / (H - 1))); s(0, 0, 0); time.sleep(h)
go(700, 386, 0.4)
s(1, 0x110, 1); s(0, 0, 0); time.sleep(0.2)
s(1, 42, 1); s(0, 0, 0); time.sleep(0.1)                           # left Shift down
for i in range(1, 9): go(700 + (tx - 700) * i // 8, 386 + (ty - 386) * i // 8)
time.sleep(hold)
s(1, 0x110, 0); s(0, 0, 0); time.sleep(0.15)
s(1, 42, 0); s(0, 0, 0); time.sleep(0.6)
fcntl.ioctl(fd, 0x5502); os.close(fd)
