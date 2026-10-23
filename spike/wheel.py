# usage: wheel.py x y dir(+1 up / -1 down): move an absolute pointer to x,y and turn the wheel once
import os, fcntl, struct, sys, time
x, y, d = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])
W, H = 1920, 1080
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
for c, a in ((0x40045564, 1), (0x40045565, 0x110), (0x40045564, 2), (0x40045566, 8), (0x40045564, 3), (0x40045567, 0), (0x40045567, 1)): fcntl.ioctl(fd, c, a)
am = [0] * 64; am[0] = am[1] = 32767
os.write(fd, struct.pack("80sHHHHi", b"kwinish-wheel-ptr", 3, 0x1234, 0x567e, 1, 0) + struct.pack("64i", *am) + b"\0" * (4 * 64 * 3))
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
s(3, 0, round(x * 32767 / (W - 1))); s(3, 1, round(y * 32767 / (H - 1))); s(0, 0, 0); time.sleep(0.5)
s(2, 8, d); s(0, 0, 0); time.sleep(0.5)
fcntl.ioctl(fd, 0x5502); os.close(fd)
