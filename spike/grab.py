# usage: grab.py move|resize ADDRESS X0 Y0 X1 Y1 : pointer to (X0,Y0), start the menu Move/Resize grab on the window,
# glide to (X1,Y1), click to end it. Prints the window rect before and after.
import os, fcntl, struct, sys, time, json, subprocess
mode, addr, x0, y0, x1, y1 = sys.argv[1], sys.argv[2], *map(int, sys.argv[3:7])
_m = json.loads(subprocess.run(["hyprctl", "monitors", "-j"], capture_output=True, text=True).stdout)
_b = [(m["x"], m["y"], m["x"] + m["width"] / m["scale"], m["y"] + m["height"] / m["scale"]) for m in _m]
OX, OY = min(b[0] for b in _b), min(b[1] for b in _b)
W, H = max(b[2] for b in _b) - OX, max(b[3] for b in _b) - OY
def rect():
    c = [c for c in json.loads(subprocess.run(["hyprctl", "clients", "-j"], capture_output=True, text=True).stdout) if c["address"] == addr][0]
    return c["at"], c["size"], c["floating"]
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
for c, a in ((0x40045564, 1), (0x40045565, 0x110), (0x40045564, 3), (0x40045567, 0), (0x40045567, 1)): fcntl.ioctl(fd, c, a)
am = [0] * 64; am[0] = am[1] = 32767
os.write(fd, struct.pack("80sHHHHi", b"kwinish-grab-ptr", 3, 0x1234, 0x5684, 1, 0) + struct.pack("64i", *am) + b"\0" * (4 * 64 * 3))
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
def go(x, y, h=0.05): s(3, 0, round((x - OX) * 32767 / (W - 1))); s(3, 1, round((y - OY) * 32767 / (H - 1))); s(0, 0, 0); time.sleep(h)
go(x0, y0, 0.4)
print("before", rect())
env = dict(os.environ)
subprocess.run(["hyprctl", "eval", f"package.loaded.kwinish.window_action('{addr}','grab:{mode}')"], capture_output=True, env=env)
time.sleep(0.3)
for i in range(1, 11): go(x0 + (x1 - x0) * i / 10, y0 + (y1 - y0) * i / 10)
print("during", rect())
s(1, 0x110, 1); s(0, 0, 0); time.sleep(0.06); s(1, 0x110, 0); s(0, 0, 0); time.sleep(0.4)
go(x1 + 40, y1 + 40)  # after the click the window must stay put
print("after click + extra move", rect())
fcntl.ioctl(fd, 0x5502); os.close(fd)
