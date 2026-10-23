# Alt down, F24 down/up, Alt up via uinput; F24 is unbound on the owner's desktop
import os, fcntl, struct, time
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
fcntl.ioctl(fd, 0x40045564, 1)
for k in range(1, 249): fcntl.ioctl(fd, 0x40045565, k)
os.write(fd, struct.pack("80sHHHHi", b"kwinish-spike-kbd", 3, 0x1234, 0x5679, 1, 0) + b"\0" * 1024)
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
def syn(): s(0, 0, 0)
for code, v in ((56, 1), (194, 1), (194, 0), (56, 0)):
    s(1, code, v); syn(); time.sleep(0.25)
fcntl.ioctl(fd, 0x5502); os.close(fd)
