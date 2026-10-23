# click-at.py X Y [BUTTON=left|right] [HOLD]: one click with a virtual pointer
import os, fcntl, struct, sys, time
W, H = 1920, 1080
x, y = int(sys.argv[1]), int(sys.argv[2]); btn = 0x111 if len(sys.argv) > 3 and sys.argv[3] == "right" else 0x110
wait = float(sys.argv[4]) if len(sys.argv) > 4 else 1.0
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
for c, a in ((0x40045564, 1), (0x40045565, 0x110), (0x40045565, 0x111), (0x40045564, 3), (0x40045567, 0), (0x40045567, 1)): fcntl.ioctl(fd, c, a)
am = [0] * 64; am[0] = am[1] = 32767
os.write(fd, struct.pack("80sHHHHi", b"kwinish-click-ptr", 3, 0x1234, 0x5683, 1, 0) + struct.pack("64i", *am) + b"\0" * (4 * 64 * 3))
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
s(3, 0, round(x * 32767 / (W - 1))); s(3, 1, round(y * 32767 / (H - 1))); s(0, 0, 0); time.sleep(0.4)
s(1, btn, 1); s(0, 0, 0); time.sleep(0.06); s(1, btn, 0); s(0, 0, 0); time.sleep(wait)
