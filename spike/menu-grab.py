# usage: menu-grab.py ADDR_BACK ADDR_FRONT BAR_X BAR_Y ITEM_X ITEM_Y DX DY
# The whole real path with a virtual pointer: right-click the bar of a covered window, click a menu entry (Move/Resize), glide the pointer
# by (DX, DY), click to end. Prints both windows' rectangles before and after.
import os, fcntl, struct, sys, time, json, subprocess
back, front = sys.argv[1], sys.argv[2]; bx, by, ix, iy, dx, dy = map(int, sys.argv[3:9])
_m = json.loads(subprocess.run(["hyprctl", "monitors", "-j"], capture_output=True, text=True).stdout)
_b = [(m["x"], m["y"], m["x"] + m["width"] / m["scale"], m["y"] + m["height"] / m["scale"]) for m in _m]
OX, OY = min(b[0] for b in _b), min(b[1] for b in _b)
W, H = max(b[2] for b in _b) - OX, max(b[3] for b in _b) - OY
def rect():
    cs = {c["address"]: c for c in json.loads(subprocess.run(["hyprctl", "clients", "-j"], capture_output=True, text=True).stdout)}
    return {n: (cs[a]["at"], cs[a]["size"]) for n, a in (("back", back), ("front", front))}
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
for c, a in ((0x40045564, 1), (0x40045565, 0x110), (0x40045565, 0x111), (0x40045564, 3), (0x40045567, 0), (0x40045567, 1)): fcntl.ioctl(fd, c, a)
am = [0] * 64; am[0] = am[1] = 32767
os.write(fd, struct.pack("80sHHHHi", b"kwinish-menugrab-ptr", 3, 0x1234, 0x5685, 1, 0) + struct.pack("64i", *am) + b"\0" * (4 * 64 * 3))
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
def go(x, y, h=0.05): s(3, 0, round((x - OX) * 32767 / (W - 1))); s(3, 1, round((y - OY) * 32767 / (H - 1))); s(0, 0, 0); time.sleep(h)
def click(btn): s(1, btn, 1); s(0, 0, 0); time.sleep(0.06); s(1, btn, 0); s(0, 0, 0)
print("before", rect())
go(bx, by, 0.4); click(0x111); time.sleep(1.0)          # right-click on the covered window's bar
go(ix, iy, 0.5); click(0x110); time.sleep(0.8)           # the menu entry
for i in range(1, 11): go(ix + dx * i / 10, iy + dy * i / 10)
print("during", rect())
click(0x110); time.sleep(0.5)
print("after ", rect())
fcntl.ioctl(fd, 0x5502); os.close(fd)
