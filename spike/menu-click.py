# open the menu at (300,200) for KW_ADDR and click one root row: menu-click.py ROW_Y
import os, fcntl, struct, subprocess, sys, time
W, H = 1920, 1080
steps = [tuple(int(v) for v in a.split(',')) for a in sys.argv[1:]]  # hover each x,y in order; the last one is clicked
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
for c, a in ((0x40045564, 1), (0x40045565, 0x110), (0x40045564, 3), (0x40045567, 0), (0x40045567, 1)): fcntl.ioctl(fd, c, a)
am = [0] * 64; am[0] = am[1] = 32767
os.write(fd, struct.pack("80sHHHHi", b"kwinish-mclick-ptr", 3, 0x1234, 0x5682, 1, 0) + struct.pack("64i", *am) + b"\0" * (4 * 64 * 3))
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
def go(px, py, wait=0.5): s(3, 0, round(px * 32767 / (W - 1))); s(3, 1, round(py * 32767 / (H - 1))); s(0, 0, 0); time.sleep(wait)
go(300, 200, 0.3)
subprocess.Popen(["setsid", "ui/target/debug/kwinish-ui", "menu", os.environ["KW_ADDR"]], env={**os.environ, "LANG": "pt_BR.UTF-8"}, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(1.4)
for hx, hy in steps: go(hx, hy, 0.6)
s(1, 0x110, 1); s(0, 0, 0); time.sleep(0.06); s(1, 0x110, 0); s(0, 0, 0); time.sleep(1.0)
fcntl.ioctl(fd, 0x5502); os.close(fd)
