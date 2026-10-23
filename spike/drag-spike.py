# R3 spike: press button over a floating throwaway kitty, dispatch drag, move, release; events go to events.log via listener.lua
import os, fcntl, struct, subprocess, time
W, H = 1920, 1080
fd = os.open("/dev/uinput", os.O_WRONLY | os.O_NONBLOCK)
for c, a in ((0x40045564, 1), (0x40045565, 0x110), (0x40045564, 3), (0x40045567, 0), (0x40045567, 1)): fcntl.ioctl(fd, c, a)
am = [0] * 64; am[0] = am[1] = 32767
os.write(fd, struct.pack("80sHHHHi", b"kwinish-spike-ptr", 3, 0x1234, 0x567b, 1, 0) + struct.pack("64i", *am) + b"\0" * (4 * 64 * 3))
fcntl.ioctl(fd, 0x5501); time.sleep(2.5)
def s(t, c, v): os.write(fd, struct.pack("llHHi", 0, 0, t, c, v))
def go(x, y, hold=0.15): s(3, 0, round(x * 32767 / (W - 1))); s(3, 1, round(y * 32767 / (H - 1))); s(0, 0, 0); time.sleep(hold)
def mark(m): open("events.log", "a").write(f"--- {m}\n")
go(800, 400, 0.5)
mark("press"); s(1, 0x110, 1); s(0, 0, 0); time.sleep(0.2)
mark("dispatch drag"); subprocess.run(["hyprctl", "dispatch", "hl.dsp.window.drag()"]); time.sleep(0.2)
for i in range(1, 9): go(800 - i * 40, 400 + i * 20, 0.08)
mark("release"); s(1, 0x110, 0); s(0, 0, 0); time.sleep(0.6)
mark("after release")
fcntl.ioctl(fd, 0x5502); os.close(fd)
