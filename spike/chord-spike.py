# usage: chord-spike.py HOLD_SECONDS code[,code...] ...  : press modifiers listed first, tap last key, hold, release modifiers
import os, fcntl, struct, sys, time
hold = float(sys.argv[1]); codes = [int(c) for c in sys.argv[2].split(",")]
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
fcntl.ioctl(fd, 0x40045564, 1)
for k in range(1, 249): fcntl.ioctl(fd, 0x40045565, k)
os.write(fd, struct.pack("80sHHHHi", b"kwinish-chord-kbd", 3, 0x1234, 0x567c, 1, 0) + b"\0" * 1024)
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(c, v): os.write(fd, struct.pack("llHHi", 0, 0, 1, c, v)); os.write(fd, struct.pack("llHHi", 0, 0, 0, 0, 0)); time.sleep(0.12)
*mods, key = codes
for m in mods: s(m, 1)
s(key, 1); s(key, 0)
time.sleep(hold)
for m in reversed(mods): s(m, 0)
fcntl.ioctl(fd, 0x5502); os.close(fd)
