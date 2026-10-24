# usage: keys.py CODE[+CODE...] ... : press each chord (Linux key codes, e.g. 125+16 = SUPER+Q) with a virtual keyboard
import os, fcntl, struct, sys, time
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
fcntl.ioctl(fd, 0x40045564, 1)
for k in range(1, 249): fcntl.ioctl(fd, 0x40045565, k)
os.write(fd, struct.pack("80sHHHHi", b"kwinish-keys-kbd", 3, 0x1234, 0x567a, 1, 0) + b"\0" * 1024)
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
for chord in sys.argv[1:]:
    codes = [int(c) for c in chord.split("+")]
    for c in codes: s(1, c, 1); s(0, 0, 0); time.sleep(0.08)
    time.sleep(0.2)
    for c in reversed(codes): s(1, c, 0); s(0, 0, 0); time.sleep(0.08)
    time.sleep(0.5)
fcntl.ioctl(fd, 0x5502); os.close(fd)
