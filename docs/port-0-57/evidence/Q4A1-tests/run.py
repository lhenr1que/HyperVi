#!/usr/bin/env python3
"""Q4-A1 runner (host-free): alternative 5 (Kwinish never requests a drag end without proof; the host ends drags on
uncancelled input). Builds GCC/Clang with -Werror and ASan/UBSan, compares the scenario x variant matrix with
expected-matrix.txt, then mutates the alt5-rc design. Exit 0 = matrix reproduced and every mutant detected by its declared
cell; it is NOT an approval. Also reruns the previous study (../Q4A-policy-tests/run.py) to show R1-R4, H1, L1 unchanged."""
import os, pathlib, shutil, subprocess, sys, tempfile

HERE = pathlib.Path(__file__).resolve().parent
PREV = HERE.parent / "Q4A-policy-tests"
SCRATCH = pathlib.Path(tempfile.mkdtemp(prefix="q4a1.", dir=os.environ.get("TMPDIR", "/home/lhen/.hermes/cache/scratch")))
ENV = dict(os.environ, HOME=str(SCRATCH / "home"), XDG_CONFIG_HOME=str(SCRATCH / "home/.config"), XDG_CACHE_HOME=str(SCRATCH / "home/.cache"))
for k in ("DISPLAY", "WAYLAND_DISPLAY", "HYPRLAND_INSTANCE_SIGNATURE"):
    ENV.pop(k, None)
(SCRATCH / "home").mkdir()
HU = subprocess.run(["pkg-config", "--cflags", "--libs", "hyprutils"], capture_output=True, text=True, check=True).stdout.split()
print(f"scratch: {SCRATCH}")


def stage(tag):
    d = SCRATCH / tag
    (d / "Q4A1-tests").mkdir(parents=True)
    shutil.copytree(PREV, d / "Q4A-policy-tests")
    shutil.copy(HERE / "q4a1test.cpp", d / "Q4A1-tests" / "q4a1test.cpp")
    return d


def build_run(d, cxx, tag):
    exe = SCRATCH / f"t-{tag}-{cxx}"
    b = subprocess.run([cxx, "-std=c++2b", "-Wall", "-Wextra", "-Werror", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                        str(d / "Q4A1-tests" / "q4a1test.cpp"), "-o", str(exe), *HU], capture_output=True, text=True, env=ENV)
    if b.returncode:
        return b.returncode, None, b.stdout + b.stderr
    return 0, subprocess.run([str(exe)], capture_output=True, text=True, env=ENV, timeout=120), ""


def matrix(out):
    L = out.splitlines()
    a = next(i for i, l in enumerate(L) if l.startswith("scenario \\ variant"))
    b = next(i for i, l in enumerate(L) if l.startswith("legend:"))
    head = L[a].split()[3:]
    return L[a:b + 1], {(l.split()[0], p): c for l in L[a + 1:b] for p, c in zip(head, l.split()[1:])}


fail = False
base = stage("base")
outs = {}
for cxx in ("g++", "clang++"):
    rc, r, log = build_run(base, cxx, "base")
    if rc or r.returncode or r.stderr.strip():
        print(f"FAIL {cxx}: build={rc} {log[:2000]} run={r and r.returncode} {r and r.stderr[:2000]}")
        sys.exit(1)
    outs[cxx] = r.stdout
    (SCRATCH / f"run-{cxx}.txt").write_text(r.stdout)
    print(f"{cxx}: build 0, run 0, no sanitizer output")
if outs["g++"] != outs["clang++"]:
    print("FAIL: GCC and Clang outputs differ"); fail = True
else:
    print("GCC and Clang outputs identical")
text, M = matrix(outs["g++"])
exp = (HERE / "expected-matrix.txt").read_text().splitlines()
if text != exp:
    print("FAIL: matrix differs from expected-matrix.txt")
    for a, b in zip(text, exp):
        if a != b: print(f"  got: {a}\n  exp: {b}")
    fail = True
else:
    print(f"matrix equals expected-matrix.txt ({len(text) - 2} scenarios x {len(text[0].split()) - 3} variants)")
print("\n".join(text))
scen = sorted({s for s, _ in M})
for v in text[0].split()[3:]:
    bad = {k: [s for s in scen if k in M[(s, v)]] for k in "1234s"}
    print(f"VERDICT {v}: " + " ".join(f"{k}={bad[k] or 'none'}" for k in "1234s"))

F = "Q4A1-tests/q4a1test.cpp"
MUT = [  # (name, file, find, replace, "SCENARIO VARIANT ORACLE"): the named oracle must newly fail in that cell
    ("alt5-cancels-bar-release", F, "if (b.cancelledDown && !rewritesInput()) info.cancelled = true;", "if (b.cancelledDown) info.cancelled = true;", "B1.bar.floating alt5-rc 2"),
    ("alt5-cancels-grab-release", F, "if (!rewritesInput()) info.cancelled = true; // alt 5", "info.cancelled = true; // alt 5", "M1.grab.click alt5-rc 2"),
    ("alt5-keeps-end-requests", F, "default: notes.push_back(std::string(\"end request dropped at \") + where); break;", "default: (void)where; (void)h.endDragTarget(); break;", "T1.third.same-window alt5-rc 1"),
    ("alt5-float-action-unguarded", F, "if (rewritesInput() && h.target()) { // alt 5 guard", "if (false) { // alt 5 guard", "F1.float-during-third alt5-rc 1"),
    ("alt5-rearm-ends-first", F, "default: (void)kwBegin(b.win); break; // begin only", "default: (void)h.endDragTarget(); (void)kwBegin(b.win); break; // begin only", "RA2.rearm.third alt5-rc 1"),
    ("alt5-no-unsnap-redesign", F, "if (rewritesInput() && b.snapped) {", "if (false) {", "US1.unsnap alt5-rc s"),
    ("alt5-unsnap-no-abort", F, "        if (h.target()) { // someone else is dragging", "        if (false) { // someone else is dragging", "US3.unsnap.third alt5-rc s"),
    ("alt5-rearm-during-deferred-unsnap", F, "if (b.dragging && !b.deferredUnsnap) kwRearm(b);", "if (b.dragging) kwRearm(b);", "US4.unsnap.shift-before-motion alt5-rc s"),
    ("alt5-bar-press-no-yield", F, "if (rewritesInput() && h.target()) { // alt 5: a drag is active", "if (false) { // alt 5: a drag is active", "Y1.bar-press-during-third alt5-rc 3"),
    ("alt5-grab-flag-kept-on-key", F, "if (rewritesInput() && grabActive && bindsAllowed) { // alt 5", "if (false) { // alt 5", "M4.grab.shift-then-third alt5-rc 3"),
    ("alt5-keeps-bar-flag", F, "            kwEnd(\"barDeco.cpp:649 release\");\n            b.dragging = false;\n", "            kwEnd(\"barDeco.cpp:649 release\");\n            if (!rewritesInput()) b.dragging = false;\n", "B1.bar.floating alt5-rc 4"),
    ("alt5-right-click-no-yield", F, "if (rewritesInput() && h.target()) { // alt 5 (review 2): yield", "if (false) { // alt 5 (review 2): yield", "R5.right-click-during-third alt5-rc 3"),
    ("alt5-grab-armed-without-drag", F, "if (rewritesInput() && r != STARTED) {", "if (false && r != STARTED) {", "M5.grab.fails-during-third alt5-rc 3"),
    ("alt5-rearm-flag-kept", F, "if (rewritesInput()) b.rearmPending = false; // alt 5 (review 2)", "// removed", "RA3.shift-then-release alt5-rc 4"),
    ("helper-recheck", "Q4A-policy-tests/policies.hpp", "        return REFUSED_BY_DECO;\n    if (h.target())\n        return ALREADY;\n", "        return REFUSED_BY_DECO;\n", "H1i.deco-start-false alt5-rc 1"),
]
killed = 0
for name, f, find, repl, killer in MUT:
    d = stage(f"mut-{name}")
    s = (d / f).read_text()
    if s.count(find) != 1:
        print(f"MUT {name}: FAIL anchor count={s.count(find)}"); fail = True; continue
    (d / f).write_text(s.replace(find, repl))
    rc, r, log = build_run(d, "g++", name)
    if rc or r.returncode or r.stderr.strip():
        print(f"MUT {name}: FAIL build/run (not a detection) {log[:600]} {r and r.stderr[:600]}"); fail = True; continue
    _, MM = matrix(r.stdout)
    diff = sorted(f"{s} {p}: {M[(s, p)]}->{MM[(s, p)]}" for (s, p) in M if M[(s, p)] != MM[(s, p)])
    ks, kp, ko = killer.split()
    if ko not in M[(ks, kp)] and ko in MM[(ks, kp)]:
        killed += 1; print(f"MUT {name}: killed by {killer}; changed: {diff}")
    else:
        fail = True; print(f"MUT {name}: FAIL declared oracle {killer} not newly failing; changed: {diff}")
print(f"mutations: {killed}/{len(MUT)} killed by declared cell")
prev = subprocess.run([sys.executable, str(PREV / "run.py")], capture_output=True, text=True, timeout=1500)
tail = [l for l in prev.stdout.splitlines() if l.startswith(("RESULT", "mutations:", "matrix equals", "VERDICT veto-rc", "VERDICT C3-prev"))]
print(f"previous study (R1-R4, H1, L1 preserved): exit {prev.returncode}\n  " + "\n  ".join(tail))
if prev.returncode:
    fail = True
print("RESULT:", "FAIL" if fail else "PASS (matrix reproduced; see VERDICT lines - this is not an approval)")
sys.exit(1 if fail else 0)
