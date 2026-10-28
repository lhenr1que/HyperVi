#!/usr/bin/env python3
"""Probe a disposable nested compositor; the parent's only eval spawns it."""
import argparse
import json
import os
from pathlib import Path
import shlex
import signal
import subprocess
import tempfile
import time

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent
RULE_CASES = {
    "rules-class": 'hl.window_rule({ name = "kwinish-nested-class", match = { class = "kwinish-nested-probe" }, float = true })',
    "rules-size": 'hl.window_rule({ name = "kwinish-nested-size", match = { workspace = "1" }, float = true, size = "819 544", center = true })',
    "rules-offset": 'hl.window_rule({ name = "kwinish-nested-offset", match = { workspace = "1" }, float = true, size = "819 544", move = "100 100" })',
}
# A fixed-size GTK4 window: Hyprland floats it on its own, like a dialog.
DIALOG = ["python3", "-I", "-c", "import gi; gi.require_version('Gtk', '4.0'); from gi.repository import Gtk; "
          "a = Gtk.Application(application_id='kwinish.nested.dialog'); "
          "a.connect('activate', lambda a: Gtk.Window(application=a, title='kwinish-nested-dialog', resizable=False, "
          "default_width=300, default_height=150).present()); a.run(None)"]


def ctl(instance, *args, check=True):
    if not instance:
        raise RuntimeError("An explicit compositor signature is required")
    return subprocess.run(["hyprctl", "-i", instance, *args], text=True,
                          capture_output=True, timeout=4, check=check).stdout


def instances():
    return json.loads(subprocess.check_output(["hyprctl", "instances", "-j"], text=True))


def clients(instance):
    return json.loads(ctl(instance, "clients", "-j"))


def snapshot(instance):
    return [{k: w[k] for k in ("address", "class", "pid", "workspace")} for w in clients(instance)]


def write_json(path, data):
    path.write_text(json.dumps(data, indent=2) + "\n")


def stop_process(pid):
    try:
        os.kill(pid, signal.SIGTERM)
    except ProcessLookupError:
        return


def probe_pids(cfg):
    result = []
    for path in Path("/proc").iterdir():
        if not path.name.isdigit():
            continue
        try:
            argv = (path / "cmdline").read_bytes().split(b"\0")
        except (FileNotFoundError, PermissionError, ProcessLookupError):
            continue
        if str(cfg).encode() in argv and Path(os.fsdecode(argv[0])).name == "Hyprland":
            result.append(int(path.name))
    return result


def stop_kitty(process):
    process.terminate()
    try:
        process.wait(timeout=4)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=4)


def run(case, out):
    owner = os.environ.get("HYPRLAND_INSTANCE_SIGNATURE")
    if not owner:
        raise RuntimeError("The parent session must be selected in the environment")
    out.mkdir(parents=True, exist_ok=False)
    before = snapshot(owner)
    write_json(out / "owner-before.json", before)
    version = subprocess.check_output(["Hyprland", "--version"], text=True, stderr=subprocess.STDOUT)
    (out / "version.txt").write_text(version)
    nested = None
    kitty = None
    root = Path(tempfile.mkdtemp(prefix="kwinish-nested-open-"))
    (out / "temporary-root.txt").write_text(str(root) + "\n")
    for name in ("home", "config/kwinish", "state", "cache", "runtime"):
        (root / name).mkdir(parents=True, exist_ok=True)
    (root / "config/kwinish/workspace-modes.json").write_text('{"1":"floating"}\n')
    # The parent is allowed to run exactly hl.exec_cmd("Hyprland -c <cfg>", rules).
    # Embed case data rather than changing its environment or registering hooks there.
    cfg = root / "hyprland.lua"
    template = (HERE / "hyprland.lua").read_text()
    template = template.replace('dofile(os.getenv("KWINISH_NESTED_CASE") or "/tmp/kwinish-nested-case.lua")',
        "\n".join(f"{key} = {json.dumps(value)}" for key, value in
                  (("NESTED_ROOT", str(root)), ("NESTED_CASE", case), ("NESTED_REPO", str(REPO)))))
    if case == "core-mode":  # the size the owner's workspace 9 gives the nested window: no resize after monitor.added
        template = template.replace('mode = "1280x800@60"', 'mode = "1827x1000@60"')
    if case == "rules-offset":  # a monitor away from the origin tells monitor-local from global rule coordinates
        template = template.replace('position = "0x0"', 'position = "500x300"')
    cfg.write_text(template)
    (out / "hyprland.lua").write_text(template)
    cmd = "Hyprland -c " + shlex.quote(str(cfg))
    spawn = "hl.exec_cmd(" + json.dumps(cmd) + ', { workspace = "9 silent" })'
    (out / "parent-spawn.lua").write_text(spawn + "\n")
    try:
        (out / "parent-spawn-result.txt").write_text(ctl(owner, "eval", spawn))
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            matches = []
            for inst in instances():
                if inst["instance"] == owner:
                    continue
                try:
                    argv = Path(f'/proc/{inst["pid"]}/cmdline').read_bytes().split(b"\0")
                except FileNotFoundError:
                    continue
                if str(cfg).encode() in argv:
                    matches.append(inst)
            if len(matches) > 1:
                raise RuntimeError("More than one compositor matches the exact probe config")
            if matches:
                nested = matches[0]
                if (root / "ready").exists():
                    break
            time.sleep(0.05)
        if nested is None or not (root / "ready").exists():
            raise RuntimeError("Nested compositor did not become ready")
        signature = nested["instance"]
        assert signature != owner
        assert nested["wl_socket"] != os.environ.get("WAYLAND_DISPLAY")
        write_json(out / "instance.json", nested)
        (out / "configerrors.txt").write_text(ctl(signature, "configerrors"))
        write_json(out / "monitors.json", json.loads(ctl(signature, "monitors", "-j")))

        def nested_env():
            env = os.environ.copy()
            env.update(HOME=str(root / "home"), XDG_CONFIG_HOME=str(root / "config"),
                       XDG_STATE_HOME=str(root / "state"), XDG_CACHE_HOME=str(root / "cache"),
                       WAYLAND_DISPLAY=nested["wl_socket"], HYPRLAND_INSTANCE_SIGNATURE=signature)
            env.pop("DISPLAY", None)
            env.pop("DBUS_SESSION_BUS_ADDRESS", None)
            return env

        def sample(label, argv=None):
            nonlocal kitty
            env = nested_env()
            start = time.monotonic()
            with (out / (label + "-kitty.log")).open("w") as stderr:
                kitty = subprocess.Popen(argv or ["kitty", "--config", "NONE", "--class", "kwinish-nested-probe",
                    "--title", "kwinish-nested-" + label, "-o", "linux_display_server=wayland",
                    "-o", "confirm_os_window_close=0", "/usr/bin/sleep", "20"], env=env,
                    stdout=stderr, stderr=stderr)
                samples = []
                next_at = start
                while time.monotonic() - start < 1.5:
                    samples.append({"ms": round((time.monotonic() - start) * 1000, 2),
                                    "clients": clients(signature)})
                    next_at += 0.015
                    time.sleep(max(0, next_at - time.monotonic()))
                write_json(out / (label + "-samples.json"), samples)
                first = next((s for s in samples if s["clients"]), None)
                if first is None:
                    raise RuntimeError("No mapped window sampled; inspect kitty log")
                write_json(out / (label + "-first.json"), first)
                stop_kitty(kitty)
                kitty = None
            deadline = time.monotonic() + 3
            while clients(signature) and time.monotonic() < deadline:
                time.sleep(0.03)
            if clients(signature):
                raise RuntimeError("Probe window did not close")

        if case == "rules":
            rule = 'hl.window_rule({ name = "kwinish-nested-floating-1", match = { workspace = "1" }, float = true })'
            (out / "rule-enable.txt").write_text(ctl(signature, "eval", rule))
            sample("enabled")
            disable = 'hl.window_rule({ name = "kwinish-nested-floating-1", enabled = false })'
            (out / "rule-disable.txt").write_text(ctl(signature, "eval", disable))
            sample("disabled")
        elif case == "rules-mode":
            # The rule the core would declare per floating workspace, redeclared by name for the cascade.
            def rule(x, y):
                return ('hl.window_rule({ name = "kwinish-nested-mode-1", match = { workspace = "1", float = false, '
                        f'fullscreen = false }}, float = true, size = "819 544", move = "{x} {y}", tag = "+kwinish-mode-floated" }})')
            kitty_fs = ["kitty", "--config", "NONE", "--class", "kwinish-nested-probe", "--start-as=fullscreen",
                        "-o", "linux_display_server=wayland", "-o", "confirm_os_window_close=0", "/usr/bin/sleep", "20"]
            sample("dialog-norule", DIALOG)
            (out / "rule-1.txt").write_text(ctl(signature, "eval", rule(100, 100)))
            sample("kitty1")
            (out / "rule-2.txt").write_text(ctl(signature, "eval", rule(124, 124)))
            sample("kitty2")
            sample("dialog", DIALOG)
            sample("fullscreen", kitty_fs)
            (out / "rule-off.txt").write_text(ctl(signature, "eval",
                'hl.window_rule({ name = "kwinish-nested-mode-1", enabled = false })'))
            sample("off")
        elif case == "core-mode":
            # The repository core with workspace 1 floating: two windows open together, a dialog, leaving the mode, one more window.
            procs, first, log = [], {}, []
            def kitty_argv(label):
                return ["kitty", "--config", "NONE", "--class", "kwinish-nested-probe", "--title", "kwinish-nested-" + label,
                        "-o", "linux_display_server=wayland", "-o", "confirm_os_window_close=0", "/usr/bin/sleep", "30"]
            def watch(seconds):
                end = time.monotonic() + seconds
                while time.monotonic() < end:
                    for c in clients(signature):
                        first.setdefault(c["address"], {k: c[k] for k in ("title", "class", "floating", "at", "size", "tags")})
                    time.sleep(0.015)
            def spawn(argv):
                procs.append(subprocess.Popen(argv, env=nested_env(), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL))
            try:
                spawn(kitty_argv("a")); watch(0.8)
                spawn(kitty_argv("b")); watch(0.8)
                spawn(DIALOG); watch(1.0)
                log.append({"step": "all open", "clients": clients(signature)})
                (out / "mode-tiling.txt").write_text(ctl(signature, "eval", 'require("kwinish").set_workspace_mode(1, "tiling")'))
                time.sleep(0.5)
                log.append({"step": "after tiling", "clients": clients(signature)})
                spawn(kitty_argv("c")); watch(0.8)
                write_json(out / "core-first.json", first)
                write_json(out / "core-steps.json", log)
            finally:
                for p in procs:
                    stop_kitty(p)
            deadline = time.monotonic() + 3
            while clients(signature) and time.monotonic() < deadline:
                time.sleep(0.03)
        elif case == "maximize":
            # What a shell reading `workspaces -j` sees for the bar's maximize button (the plugin's `fullscreen 1`).
            proc = subprocess.Popen(["kitty", "--config", "NONE", "--class", "kwinish-nested-probe", "-o", "linux_display_server=wayland",
                                     "-o", "confirm_os_window_close=0", "/usr/bin/sleep", "20"], env=nested_env(),
                                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            try:
                deadline = time.monotonic() + 5
                while not clients(signature) and time.monotonic() < deadline:
                    time.sleep(0.03)
                steps = []
                # The core's menu path; the plugin calls the same toggle in-process as the `fullscreen 1` dispatcher.
                for label, mode in (("none", None), ("maximized", "maximized"), ("back", "maximized"), ("fullscreen", "fullscreen"), ("off", "fullscreen")):
                    if mode is not None:
                        ctl(signature, "eval", 'hl.dispatch(hl.dsp.window.fullscreen({ mode = "%s" }))' % mode)
                        time.sleep(0.4)
                    steps.append({"step": label, "workspaces": [{k: w[k] for k in ("name", "hasfullscreen")}
                                                                 for w in json.loads(ctl(signature, "workspaces", "-j"))],
                                  "clients": [{k: c[k] for k in ("fullscreen", "fullscreenClient", "at", "size")}
                                              for c in clients(signature)]})
                write_json(out / "maximize-steps.json", steps)
            finally:
                stop_kitty(proc)
            deadline = time.monotonic() + 3
            while clients(signature) and time.monotonic() < deadline:
                time.sleep(0.03)
        elif case in RULE_CASES:
            # One rule per run: by class (known before the first configure) or by workspace with a fixed size.
            (out / "rule-enable.txt").write_text(ctl(signature, "eval", RULE_CASES[case]))
            sample(case)
        else:
            sample(case)
        (out / "survived.txt").write_text(ctl(signature, "version"))
    finally:
        if kitty is not None:
            stop_kitty(kitty)
        if nested is not None:
            try:
                ctl(nested["instance"], "dispatch", "exit", check=False)
            except (subprocess.TimeoutExpired, OSError):
                pass
            deadline = time.monotonic() + 4
            while any(i["instance"] == nested["instance"] for i in instances()) and time.monotonic() < deadline:
                time.sleep(0.05)
            if any(i["instance"] == nested["instance"] for i in instances()):
                stop_process(nested["pid"])
        # Covers startup failure before a socket/instance was published.
        for pid in probe_pids(cfg):
            stop_process(pid)
        deadline = time.monotonic() + 3
        while probe_pids(cfg) and time.monotonic() < deadline:
            time.sleep(0.05)
        for pid in probe_pids(cfg):
            try:
                os.kill(pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
        for name in ("events.jsonl",):
            if (root / name).exists():
                (out / name).write_bytes((root / name).read_bytes())
        # Instance removal precedes the parent's handling of the Wayland close.
        # Wait for this probe's window to disappear without touching the parent.
        deadline = time.monotonic() + 4
        after = snapshot(owner)
        while nested and any(w["pid"] == nested["pid"] for w in after) and time.monotonic() < deadline:
            time.sleep(0.05)
            after = snapshot(owner)
        write_json(out / "owner-after.json", after)
        before_addresses = {w["address"] for w in before}
        added = [w for w in after if w["address"] not in before_addresses]
        nested_left = nested and any(i["instance"] == nested["instance"] for i in instances())
        write_json(out / "cleanup.json", {"added_owner_clients": added, "nested_left": bool(nested_left),
            "existing_kitty_preserved": all(w in after for w in before if w["class"] == "kitty")})
        if added or nested_left:
            raise RuntimeError("Probe cleanup failed; inspect cleanup.json")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("case", choices=("baseline", "early-log", "early-mutate", "rules", "rules-mode", "core", "core-mode", "maximize", *RULE_CASES))
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    run(args.case, args.out.resolve())
