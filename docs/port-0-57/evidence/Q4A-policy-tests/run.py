#!/usr/bin/env python3
"""Q4-A end-policy runner (host-free). Builds GCC and Clang with ASan/UBSan and -Werror, compares the policy x scenario
matrix with expected-matrix.txt, checks both compilers print identical output, then runs mutations of the candidate.
Writes nothing into the repository; scratch dir is printed. Exit 0 = observed matrix equals the expected one AND every
non-redundant mutant was detected by its declared killer cell. Exit 0 does NOT mean the candidate is approved: read the
VERDICT lines."""
import os, pathlib, re, shutil, subprocess, sys, tempfile

HERE = pathlib.Path(__file__).resolve().parent
SCRATCH = pathlib.Path(tempfile.mkdtemp(prefix="q4a-policy.", dir=os.environ.get("TMPDIR", "/home/lhen/.hermes/cache/scratch")))
ENV = dict(os.environ, HOME=str(SCRATCH / "home"), XDG_CONFIG_HOME=str(SCRATCH / "home/.config"), XDG_CACHE_HOME=str(SCRATCH / "home/.cache"))
for k in ("DISPLAY", "WAYLAND_DISPLAY", "HYPRLAND_INSTANCE_SIGNATURE"):
    ENV.pop(k, None)
(SCRATCH / "home").mkdir()
HU = subprocess.run(["pkg-config", "--cflags", "--libs", "hyprutils"], capture_output=True, text=True, check=True).stdout.split()
FILES = ["host.hpp", "policies.hpp", "policytest.cpp"]
print(f"scratch: {SCRATCH}")


def build_run(src: pathlib.Path, cxx: str, tag: str):
    exe = SCRATCH / f"t-{tag}-{cxx}"
    b = subprocess.run([cxx, "-std=c++2b", "-Wall", "-Wextra", "-Werror", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                        str(src / "policytest.cpp"), "-o", str(exe), *HU], capture_output=True, text=True, env=ENV)
    if b.returncode:
        return b.returncode, None, b.stdout + b.stderr
    r = subprocess.run([str(exe)], capture_output=True, text=True, env=ENV, timeout=120)
    return 0, r, ""


def matrix(out: str):
    lines = out.splitlines()
    start = next(i for i, l in enumerate(lines) if l.startswith("scenario \\ policy"))
    end = next(i for i, l in enumerate(lines) if l.startswith("legend:"))
    head = lines[start].split()[3:]
    m = {}
    for l in lines[start + 1:end]:
        f = l.split()
        for pol, cell in zip(head, f[1:]):
            m[(f[0], pol)] = cell
    return lines[start:end + 1], m


fail = False
outs = {}
for cxx in ("g++", "clang++"):
    rc, r, log = build_run(HERE, cxx, "base")
    if rc:
        print(f"FAIL: build {cxx} rc={rc}\n{log}")
        sys.exit(1)
    (SCRATCH / f"run-{cxx}.txt").write_text(r.stdout + r.stderr)
    if r.returncode or r.stderr.strip():
        print(f"FAIL: run {cxx} rc={r.returncode} stderr={r.stderr[:2000]}")
        sys.exit(1)
    outs[cxx] = r.stdout
    print(f"{cxx}: build 0, run 0, no sanitizer output")
if outs["g++"] != outs["clang++"]:
    print("FAIL: GCC and Clang outputs differ")
    fail = True
else:
    print("GCC and Clang outputs identical")
# ---- real installed headers: the observations used by the candidate exist with these types ----
W = HERE.parents[3]
PF = ["-std=c++2b", "-Wno-c++11-narrowing", f"-I{W / 'plugin'}"] + subprocess.run(
    ["pkg-config", "--cflags", "pixman-1", "libdrm", "hyprland", "libinput", "libudev", "wayland-server", "xkbcommon", "cairo", "librsvg-2.0"],
    capture_output=True, text=True, check=True).stdout.split()
for cxx in ("g++", "clang++"):
    pos = subprocess.run([cxx, *PF, "-fsyntax-only", str(HERE / "probe.cpp")], capture_output=True, text=True, env=ENV)
    neg = subprocess.run([cxx, *PF, "-DNEG", "-fsyntax-only", str(HERE / "probe.cpp")], capture_output=True, text=True, env=ENV)
    errs = [l for l in (neg.stdout + neg.stderr).splitlines() if re.search(r": (fatal )?error:", l)]
    ok = pos.returncode == 0 and neg.returncode != 0 and len(errs) == 1 and "sessionId" in errs[0]
    print(f"{cxx} probe: positive rc={pos.returncode}, NEG rc={neg.returncode}, NEG errors={len(errs)} -> {'OK' if ok else 'FAIL'}")
    if not ok:
        print(pos.stderr[:1500], neg.stderr[:1500])
        fail = True

text, M = matrix(outs["g++"])
expected = (HERE / "expected-matrix.txt").read_text().splitlines()
if text != expected:
    print("FAIL: matrix differs from expected-matrix.txt")
    for a, b in zip(text, expected):
        if a != b:
            print(f"  got: {a}\n  exp: {b}")
    fail = True
else:
    print(f"matrix equals expected-matrix.txt ({len(text) - 2} scenarios x {len(text[0].split()) - 3} policies)")
print("\n".join(text))

# ---- verdict lines (computed, not hard-coded) ----
pols = ["none", "target-eq", "C3-prev", "veto", "veto-rc", "defer", "hook"]
scen = sorted({s for s, _ in M})
for p in pols:
    bad = {k: [s for s in scen if k in M[(s, p)]] for k in "123s"}
    print(f"VERDICT {p}: O1(improper end)={bad['1'] or 'none'} O2(safe own end missed)={bad['2'] or 'none'} "
          f"O3(final state)={bad['3'] or 'none'} spec={bad['s'] or 'none'}")

# ---- mutations of the candidate (PolVeto and its deferred variant) ----
# (name, file, find, replace, killer cell "SCENARIO policy" or None = expected redundant)
MUT = [
    ("end-ambiguity-veto", "policies.hpp", "        if (ambiguous()) { ++vetoes; return false; }                       // (d)\n", "", "R4.host.focus veto"),
    ("begin-ambiguity-veto", "policies.hpp", "if (ambiguous()) { ++vetoes; return REFUSED; }", "if (false) { ++vetoes; return REFUSED; }", "B1.kwbegin-inside-end veto"),
    ("consume-before-end", "policies.hpp", "        disarm();                                                          // (e) consume before the call\n", "", None),
    ("end-nonreentrant-guard", "policies.hpp", "if (inEnd || inBegin) return false;                                // (e)", "if (false) return false;", None),
    ("consume-and-end-guard", "policies.hpp", "        if (inEnd || inBegin) return false;                                // (e)\n        if (ambiguous()) { ++vetoes; return false; }                       // (d)\n        if (!owns()) return false;\n        disarm();                                                          // (e) consume before the call\n",
     "        if (ambiguous()) { ++vetoes; return false; }                       // (d)\n        if (!owns()) return false;\n", "R4.emit.own veto"),
    ("callback-token", "policies.hpp", "if (tok != acquisition) return;                                // (b) stale snapshot callback", "(void)tok;", "R2.nested-snapshots veto"),
    ("disarm-bumps-token", "policies.hpp", "disarm() { ++acquisition; ownedAt.reset(); ownedTarget.reset(); listener.reset(); } // veto", "disarm() { ownedAt.reset(); ownedTarget.reset(); listener.reset(); } // veto", None),
    ("result-aware-acquisition", "policies.hpp", "if (r != STARTED) { disarm(); return r; }", "if (false) { disarm(); return r; }", "G1.kwbegin-groupbar-same veto"),
    ("generation-in-postcheck", "policies.hpp", "if (generation != G0 || h.target() != t) { disarm(); return REFUSED; } // (a)", "if (h.target() != t) { disarm(); return REFUSED; } (void)G0; // (a)", "X4.fs-reject-callback veto"),
    ("begin-nonreentrant", "policies.hpp", "if (unloading || inBegin || inEnd) return REFUSED;                 // (e)", "if (unloading || inEnd) return REFUSED;", "R1.other veto"),
    ("owns-target-alive-equal", "policies.hpp", "        if (ownedTarget.expired() || h.target() != ownedTarget.lock()) return false; // (c)\n", "", "X1.silent-death veto"),
    ("owns-listener-generation", "policies.hpp", "if (!listener || !ownedAt || *ownedAt != generation) return false;     // (b)", "if (!ownedAt) return false;", None),
    ("unload-flag", "policies.hpp", "void unload() override { unloading = true; (void)endOwned(); disarm(); }", "void unload() override { (void)endOwned(); disarm(); }", "U3.unload.then-begin veto"),
    ("defer-executes-synchronously", "policies.hpp", "        pendingFor = acquisition;\n        schedule();\n    }\n    void requestRearm", "        (void)endOwned();\n    }\n    void requestRearm", "R4.emit.host defer"),
    ("helper-target-recheck", "policies.hpp", "        return REFUSED_BY_DECO;\n    if (h.target())\n        return ALREADY;\n", "        return REFUSED_BY_DECO;\n", "H1.helper.deco-start-false veto-rc"),
    ("defer-token-recheck", "policies.hpp", "if (!tok || *tok != acquisition) return; // ownership changed since the request", "if (!tok) return;", None),
]
killed = redundant_ok = 0
for name, f, find, repl, killer in MUT:
    d = SCRATCH / f"mut-{name}"
    d.mkdir()
    for x in FILES:
        shutil.copy(HERE / x, d / x)
    s = (d / f).read_text()
    if s.count(find) != 1:
        print(f"MUT {name}: FAIL anchor count={s.count(find)}")
        fail = True
        continue
    (d / f).write_text(s.replace(find, repl))
    rc, r, log = build_run(d, "g++", name)
    if rc:
        print(f"MUT {name}: FAIL does not compile (not a detection)\n{log[:800]}")
        fail = True
        continue
    if r.returncode or r.stderr.strip():
        print(f"MUT {name}: FAIL crashed/sanitizer (not a detection): {r.stderr[:400]}")
        fail = True
        continue
    _, MM = matrix(r.stdout)
    diff = sorted(f"{s} {p}: {M[(s, p)]}->{MM[(s, p)]}" for (s, p) in M if M[(s, p)] != MM.get((s, p)))
    if killer is None:
        status = "survives (declared redundant)" if not diff else "KILLED although declared redundant"
        if diff:
            fail = True
        else:
            redundant_ok += 1
    else:
        ks, kp = killer.split()
        if M[(ks, kp)] != MM[(ks, kp)]:
            status = f"killed by {killer}"
            killed += 1
        else:
            status = f"FAIL: declared killer {killer} unchanged"
            fail = True
    print(f"MUT {name}: {status}; changed cells: {diff if diff else 'none'}")
print(f"mutations: {killed} killed by declared cell, {redundant_ok} survive as declared redundant, of {len(MUT)}")
print("RESULT:", "FAIL" if fail else "PASS (matrix reproduced; see VERDICT lines - this is not an approval)")
sys.exit(1 if fail else 0)
