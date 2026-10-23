# usage: hold-drag.py left|right|top SECONDS : press the bar of the test window (700,386), drag to the edge, hold, release
import os, fcntl, struct, sys, time
edge, hold = sys.argv[1], float(sys.argv[2])
W, H = 1920, 1080
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
for c, a in ((0x40045564, 1), (0x40045565, 0x110), (0x40045564, 3), (0x40045567, 0), (0x40045567, 1)): fcntl.ioctl(fd, c, a)
am = [0] * 64; am[0] = am[1] = 32767
os.write(fd, struct.pack("80sHHHHi", b"kwinish-hold-ptr", 3, 0x1234, 0x5680, 1, 0) + struct.pack("64i", *am) + b"\0" * (4 * 64 * 3))
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
def go(x, y, h=0.06): s(3, 0, round(x * 32767 / (W - 1))); s(3, 1, round(y * 32767 / (H - 1))); s(0, 0, 0); time.sleep(h)
path = {"left": [(640, 330), (500, 300), (300, 250), (100, 230), (20, 230), (1, 230)],
        "right": [(800, 330), (1100, 300), (1500, 250), (1800, 230), (1900, 230), (1919, 230)],
        "top": [(720, 300), (760, 200), (820, 100), (900, 40), (930, 10), (930, 2)]}[edge]
go(700, 386, 0.4)
s(1, 0x110, 1); s(0, 0, 0); time.sleep(0.2)
for x, y in path: go(x, y)
time.sleep(hold)
s(1, 0x110, 0); s(0, 0, 0); time.sleep(0.6)
fcntl.ioctl(fd, 0x5502); os.close(fd)
