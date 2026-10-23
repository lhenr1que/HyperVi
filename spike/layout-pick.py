# open `kwinish-ui layouts` at (300,200), screenshot it, then click the row at y=ROW
import os, fcntl, struct, subprocess, sys, time
W, H = 1920, 1080
row = int(sys.argv[1])
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
for c, a in ((0x40045564, 1), (0x40045565, 0x110), (0x40045564, 3), (0x40045567, 0), (0x40045567, 1)): fcntl.ioctl(fd, c, a)
am = [0] * 64; am[0] = am[1] = 32767
os.write(fd, struct.pack("80sHHHHi", b"kwinish-pick-ptr", 3, 0x1234, 0x5684, 1, 0) + struct.pack("64i", *am) + b"\0" * (4 * 64 * 3))
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
def go(x, y, wait=0.5): s(3, 0, round(x * 32767 / (W - 1))); s(3, 1, round(y * 32767 / (H - 1))); s(0, 0, 0); time.sleep(wait)
go(300, 200, 0.3)
subprocess.Popen(["setsid", "ui/target/debug/kwinish-ui", "layouts"], env={**os.environ, "LANG": "pt_BR.UTF-8"}, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(1.4)
subprocess.run(["grim", "-g", "280,180 420x290", "/tmp/claude-1000/lay.png"])
go(380, row, 0.6); s(1, 0x110, 1); s(0, 0, 0); time.sleep(0.06); s(1, 0x110, 0); s(0, 0, 0); time.sleep(1.0)
fcntl.ioctl(fd, 0x5502); os.close(fd)
