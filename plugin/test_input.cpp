// SPDX-License-Identifier: MIT
// Part of kwinish (Copyright (c) 2026 lhen).
// Tests for barInput.hpp (no Hyprland): bash tools/test-plugin-input.sh
// Built with -DKW_BASELINE against docs/port-0-57/evidence/T4-tests/baseline-barInput.hpp it must fail (T4 red).
#include <barInput.hpp> // angle brackets: only the -I directory, so the baseline copy can stand in for it

#include <chrono>
#include <cstdio>
#include <string>

using namespace KwIn;
using clk = std::chrono::steady_clock;

static int fails = 0;

static void check(bool okay, const std::string& what) {
    std::printf("%s - %s\n", okay ? "ok" : "FALHOU", what.c_str());
    fails += okay ? 0 : 1;
}

#ifdef KW_BASELINE
// The drag the baseline started or ended. Since alpha-1 SClicks has no drag state, so this would always be 0 (achado 5):
// whether a press starts a drag is barDeco.cpp's (onTitlePress), checked by tools/test-plugin-input.sh.
static int drags(const SClicks& c) {
    return c.dragStarts + c.dragEnds + c.snaps + c.dragging + c.pending;
}
#endif
static void motion(SClicks& c) {
#ifdef KW_BASELINE
    c.motion();
#else
    (void)c;
#endif
}
static void facts(bool focused, bool floating, bool maximized) {
#ifdef KW_BASELINE
    g_focused = focused, g_floating = floating, g_maximized = maximized;
#else
    (void)focused, (void)floating, (void)maximized;
#endif
}

static const clk::time_point T0{};
static SPress down(eHit hit, uint32_t button = BTN_LEFT, bool valid = true, bool taken = false) {
    return {button, true, taken, valid, hit};
}
static bool same(SResult r, eAction a, bool cancel) {
    return r.action == a && r.cancel == cancel;
}

// One bar's press and the release of a button, as the plugin wires them: presses through the bar (barDeco.cpp), releases
// through the one listener that outlives the bars (main.cpp). The baseline handled both in the bar.
#ifdef KW_BASELINE
struct SPairs {};
static SResult press(SClicks& c, SPairs&, const SPress& e, bool dbl = true, clk::time_point now = T0) {
    return c.onButton(e, dbl, now);
}
static SResult release(SClicks* c, SPairs&, uint32_t button = BTN_LEFT, bool valid = true) {
    return c ? c->onButton({button, false, false, valid, eHit::OUTSIDE}, true, T0) : SResult{}; // no bar: nobody takes it
}
#else
static SResult press(SClicks& c, SPairs& pairs, const SPress& e, bool dbl = true, clk::time_point now = T0) {
    return c.onPress(e, pairs, dbl, now);
}
static SResult release(SClicks*, SPairs& pairs, uint32_t button = BTN_LEFT, bool = true) {
    return pairs.onRelease(button);
}
#endif

// spy for apply(): what onMouseButton does with a decision, in order
struct SSpy {
    bool        floating = true;
    std::string log;
    void        cancel() { log += "cancel "; }
    void        focus() { log += "focus "; }
    void        raiseIfFloating() { log += floating ? "raise " : ""; }
    void        run(eAction a) { log += "run" + std::to_string((int)a) + " "; }
};
static std::string applied(SResult r, bool floating = true) {
    SSpy s;
    s.floating = floating;
    apply(r, s);
    return s.log;
}

int main() {
    using namespace std::chrono_literals;

    // a press on the title is a FOCUS decision taken in pairs, floating, tiled or maximized (the baseline also: no drag)
    for (const auto& [floating, maximized, name] : {std::tuple{true, false, "floating"}, {false, false, "tiled"}, {true, true, "maximized"}}) {
        facts(true, floating, maximized);
        SClicks c;
        SPairs  g;
        const auto P = press(c, g, down(eHit::TITLE), true, T0);
        motion(c);
        const auto R = release(&c, g);
        check(same(P, eAction::FOCUS, true), std::string(name) + ": title press focuses and is taken");
        check(same(R, eAction::NONE, true), std::string(name) + ": its release is taken too");
#ifdef KW_BASELINE
        check(drags(c) == 0, std::string(name) + ": no drag started, ended or snapped");
#endif
    }
    facts(true, true, false);

    // pairs: every press a bar takes has its release taken, wherever focus or the pointer went
    {
        SClicks c;
        SPairs  g;
        check(same(press(c, g, down(eHit::BUTTON), true, T0), eAction::BUTTON, true), "button press runs the button once");
        facts(false, true, false); // minimize moved focus away
        check(same(release(&c, g, BTN_LEFT, false), eAction::NONE, true), "release after minimize (focus gone, pointer off) is taken");
        facts(true, true, false);
        check(same(release(&c, g), eAction::NONE, false), "a second release is not ours");
    }
    {
        SClicks c;
        SPairs  g;
        check(same(press(c, g, down(eHit::TITLE, BTN_RIGHT), true, T0), eAction::BAR_MENU, true), "right press on the bar opens the menu");
        check(same(release(&c, g, BTN_RIGHT), eAction::NONE, true), "right release goes with it");
        check(same(press(c, g, down(eHit::BUTTON, BTN_RIGHT), true, T0), eAction::BAR_MENU, true), "right press on a button opens the menu too");
        check(same(release(&c, g, BTN_RIGHT, false), eAction::NONE, true), "right release off the bar still goes with it");
    }
    {
        SClicks c;
        SPairs  g;
        check(same(press(c, g, down(eHit::ICON), true, T0), eAction::ICON_MENU, true), "icon press opens the menu under the icon");
        check(same(release(&c, g, BTN_LEFT, false), eAction::NONE, true), "icon release (pointer now on the menu) is taken");
#ifdef KW_BASELINE
        check(drags(c) == 0, "icon: no drag");
#endif
    }

    // presses the bar does not take pass in pairs
    {
        SClicks c;
        SPairs  g;
        check(same(press(c, g, down(eHit::OUTSIDE), true, T0), eAction::NONE, false), "press off the bar passes");
        check(same(release(&c, g), eAction::NONE, false), "and its release passes");
        check(same(press(c, g, down(eHit::TITLE, BTN_LEFT, false), true, T0), eAction::NONE, false), "press where the bar takes no input passes");
        check(same(release(&c, g), eAction::NONE, false), "and its release passes");
        check(same(press(c, g, down(eHit::TITLE, BTN_MIDDLE), true, T0), eAction::NONE, false), "middle press passes");
        check(same(release(&c, g, BTN_MIDDLE), eAction::NONE, false), "middle release passes");
        check(same(press(c, g, down(eHit::BUTTON, BTN_SIDE), true, T0), eAction::NONE, false), "side button passes, even on a button");
        check(same(release(&c, g, BTN_SIDE), eAction::NONE, false), "side release passes");
        check(same(release(&c, g), eAction::NONE, false), "a release with no press passes");
        check(same(release(&c, g, BTN_RIGHT), eAction::NONE, false), "a right release with no press passes");
    }

    // two buttons at once: each pair is its own
    {
        SClicks c;
        SPairs  g;
        press(c, g, down(eHit::TITLE), true, T0);
        check(same(press(c, g, down(eHit::OUTSIDE, BTN_RIGHT), true, T0), eAction::NONE, false), "right press off the bar while left is held passes");
        check(same(release(&c, g, BTN_RIGHT), eAction::NONE, false), "its release passes");
        check(same(release(&c, g), eAction::NONE, true), "the left release is still taken");
    }

    // overlapping bars: the first takes the press, the other leaves it and its release alone
    {
        SClicks a, b;
        SPairs  g;
        const auto PA = press(a, g, down(eHit::BUTTON), true, T0);
        const auto PB = press(b, g, down(eHit::BUTTON, BTN_LEFT, true, PA.cancel), true, T0);
        check(same(PA, eAction::BUTTON, true) && same(PB, eAction::NONE, false), "overlapping bars: one action");
        const auto RA = release(&a, g), RB = release(&b, g);
        check(RA.cancel && !RB.cancel, "overlapping bars: the release is taken once");
    }

    // the bar that took the press is gone before the release (close button): the release is still taken
    {
        SPairs g;
        {
            SClicks c;
            check(same(press(c, g, down(eHit::BUTTON), true, T0), eAction::BUTTON, true), "close button press");
        }
        check(same(release(nullptr, g), eAction::NONE, true), "release after the bar is destroyed is taken");
        check(same(release(nullptr, g), eAction::NONE, false), "and only that one");
        SClicks c;
        press(c, g, down(eHit::TITLE, BTN_RIGHT), true, T0);
        check(same(release(nullptr, g, BTN_RIGHT), eAction::NONE, true), "right release after the bar is destroyed is taken");
    }

    // what onMouseButton runs for each decision (apply): cancel first, then focus, raise only for left presses on floating
    // windows, then the action once; a release is only cancelled
    {
        SClicks c;
        SPairs  g;
        const auto B = press(c, g, down(eHit::BUTTON), true, T0);
        check(applied(B) == "cancel focus raise run1 ", "button on floating: cancel, focus, raise, run once (" + applied(B) + ")");
        check(applied(B, false) == "cancel focus run1 ", "button on tiled: no raise (" + applied(B, false) + ")");
        check(applied(release(&c, g)) == "cancel ", "release: only cancelled");
        const auto M = press(c, g, down(eHit::TITLE, BTN_RIGHT), true, T0);
        check(applied(M) == "cancel focus run3 ", "right click: focus without raise, then the menu (" + applied(M) + ")");
        release(&c, g, BTN_RIGHT);
        check(applied(press(c, g, down(eHit::ICON), true, T0)) == "cancel focus raise run2 ", "icon: menu under the icon");
        release(&c, g);
        check(applied(press(c, g, down(eHit::TITLE), true, T0)) == "cancel focus raise run5 ", "title: focus and raise only (no drag)");
        release(&c, g);
        check(applied(press(c, g, down(eHit::TITLE), true, T0 + 100ms)) == "cancel focus raise run4 ", "double click: runs on_double_click");
        release(&c, g);
        check(applied(press(c, g, down(eHit::OUTSIDE), true, T0)).empty(), "press off the bar: nothing at all");
        check(applied(release(&c, g)).empty(), "its release: nothing at all");
    }

    // double click on the title (on_double_click), unchanged
    {
        SClicks c;
        SPairs  g;
        press(c, g, down(eHit::TITLE), true, T0);
        release(&c, g);
        check(same(press(c, g, down(eHit::TITLE), true, T0 + 300ms), eAction::DOUBLE_CLICK, true), "second press within 400 ms is a double click");
        release(&c, g);
        check(same(press(c, g, down(eHit::TITLE), true, T0 + 399ms), eAction::DOUBLE_CLICK, true), "a third quick press is another double (as before)");
        release(&c, g);
        check(same(press(c, g, down(eHit::TITLE), true, T0 + 1s), eAction::FOCUS, true), "after 400 ms it is a plain press again");
        release(&c, g);
        check(same(press(c, g, down(eHit::TITLE), false, T0 + 1100ms), eAction::FOCUS, true), "on_double_click empty: never a double click");
        release(&c, g);
#ifdef KW_BASELINE
        check(drags(c) == 0, "double click: no drag");
#endif
    }
    {
        SClicks c;
        SPairs  g;
        press(c, g, down(eHit::BUTTON), true, T0);
        release(&c, g);
        check(same(press(c, g, down(eHit::TITLE), true, T0 + 100ms), eAction::FOCUS, true), "a button press does not count toward a double click");
    }

    // Lua grab(mode, address): bad arguments keep their messages, a valid request is refused (no silent return)
    check(grabError(false, true, "") == "grab: expected (\"move\"|\"resize\", address)", "grab without a mode");
    check(grabError(true, false, "move") == "grab: expected (\"move\"|\"resize\", address)", "grab without an address");
    check(grabError(true, true, "spin") == "grab: mode must be move or resize", "grab with an unknown mode");
#ifdef KW_BASELINE
    check(grabError(true, true, "move").find("unavailable") != std::string::npos, "grab move is refused as unavailable");
    check(grabError(true, true, "resize").find("unavailable") != std::string::npos, "grab resize is refused as unavailable");
#else
    // alpha-1 (openspec enable-titlebar-drag-alpha1, alt5-rc): a valid grab goes ahead; a start the compositor refused is reported
    check(grabError(true, true, "move").empty(), "alpha-1: grab move is accepted");
    check(grabError(true, true, "resize").empty(), "alpha-1: grab resize is accepted");
    check(grabRefused().starts_with("grab: "), "alpha-1: a refused start is reported");

    // the bar yields to any active drag: no press taken, no menu, its release passes
    for (const auto& [hit, button, name] : {std::tuple{eHit::TITLE, BTN_LEFT, "title"}, {eHit::BUTTON, BTN_LEFT, "button"}, {eHit::ICON, BTN_LEFT, "icon"},
                                            {eHit::TITLE, BTN_RIGHT, "right on the title"}}) {
        SClicks c;
        SPairs  g;
        SPress  e    = down(hit, button);
        e.dragActive = true;
        check(same(press(c, g, e, true, T0), eAction::NONE, false), std::string("alpha-1: drag active, ") + name + " press is not taken");
        check(same(release(&c, g, button), eAction::NONE, false), std::string("alpha-1: drag active, ") + name + " release passes");
    }

    // a title press that becomes a drag: its release reaches the compositor
    {
        SClicks c;
        SPairs  g;
        check(same(press(c, g, down(eHit::TITLE), true, T0), eAction::FOCUS, true), "alpha-1: title press is taken (it may become a drag)");
        g.dragStarted(BTN_LEFT);
        check(same(release(&c, g), eAction::NONE, false), "alpha-1: once the drag started, its release passes to the compositor");
        check(same(press(c, g, down(eHit::TITLE), true, T0 + 1s), eAction::FOCUS, true), "alpha-1: a later title click is a click again");
        check(same(release(&c, g), eAction::NONE, true), "alpha-1: and its release is taken");
    }

    // the bar's drag session (alt5-rc): started only when it really started, never ended by the bar
    {
        SDrag d;
        d.start(false);
        check(!d.active, "alpha-1: a refused start arms nothing");
        d.start(true);
        check(d.active, "alpha-1: a started drag is the bar's");
        check(d.onRelease(true) == eDragStep::WAIT && d.active, "alpha-1: release with the drag going: wait for the compositor to end it");
        check(d.onEnded() == eDragStep::SNAP && !d.active, "alpha-1: the compositor ended it after the release: snap, then clear");
        check(d.onEnded() == eDragStep::NONE, "alpha-1: another drag ending is not the bar's");
    }
    {
        SDrag d;
        d.start(true);
        check(d.onRelease(false) == eDragStep::CLEAR && !d.active, "alpha-1: release when the drag already ended: clear, no snap");
    }
    {
        SDrag d;
        d.start(true);
        check(d.onEnded() == eDragStep::CLEAR && !d.active, "alpha-1: ended before the release (a key, another component): clear, no snap");
    }
    {
        SDrag d;
        d.onShift();
        check(!d.rearm, "alpha-1: Shift with no bar drag arms nothing");
        check(d.onMotion(false, true, false) == eDragStep::NONE, "alpha-1: and motion restarts nothing");
        d.start(true);
        d.onShift();
        check(d.rearm, "alpha-1: Shift mid-drag arms the rearm");
        check(d.onMotion(true, true, false) == eDragStep::NONE && d.rearm, "alpha-1: drag still going: no restart, nothing ended");
        check(d.onEnded() == eDragStep::KEEP && d.active, "alpha-1: the compositor stopped it after Shift: kept for the rearm");
        check(d.onMotion(false, false, false) == eDragStep::NONE, "alpha-1: pointer off the window: no restart");
        check(d.onMotion(false, true, true) == eDragStep::NONE, "alpha-1: unsnap pending: no restart");
        check(d.onMotion(false, true, false) == eDragStep::RESTART && !d.rearm, "alpha-1: rearm only starts again");
        check(d.onMotion(false, true, false) == eDragStep::NONE, "alpha-1: once");
        check(d.onRelease(false) == eDragStep::CLEAR && !d.active && !d.rearm, "alpha-1: release clears the rearm");
    }

    // unsnap ease before the drag starts: 300x200 back to 800x600, pointer at 25% of the bar, 10 px below its top
    {
        const auto near = [](double a, double b) { return a - b < 0.01 && b - a < 0.01; };
        SUnsnap    u;
        u.start(300, 200, 800, 600, 0.25, 10, T0);
        check(u.active, "unsnap: the ease starts");
        auto st = u.step(1000, 500, T0);
        check(near(st.w, 300) && near(st.h, 200) && !st.done, "unsnap: at 0 ms the snapped size");
        check(near(st.x, 1000 - 0.25 * 300) && near(st.y, 490), "unsnap: the pointer stays at the same fraction of the bar");
        st = u.step(1000, 500, T0 + 70ms); // ease-out cubic: 1 - 0.5^3 = 0.875
        check(near(st.w, 300 + 0.875 * 500) && near(st.h, 200 + 0.875 * 400) && !st.done && u.active, "unsnap: halfway in time, 87.5% of the way");
        check(near(st.x, 1000 - 0.25 * st.w), "unsnap: the window follows the pointer while it eases");
        st = u.step(1200, 300, T0 + 140ms);
        check(near(st.w, 800) && near(st.h, 600) && st.done && !u.active, "unsnap: at 140 ms the earlier size, done (the drag may start)");
        check(near(st.x, 1200 - 200) && near(st.y, 290), "unsnap: final position under the pointer");
        u.start(300, 200, 800, 600, 0.25, 10, T0);
        st = u.finish(1000, 500);
        check(near(st.w, 800) && near(st.h, 600) && !u.active, "unsnap: released mid-ease, lands on the earlier size");
    }

    // guard-titlebar-drag-start: the gate before any geometry or focus change on the way to the bar's drag
    check(startGate(false, true) == eStartGate::GO, "gate: no drag, pointer on the bar's window: go");
    check(startGate(false, false) == eStartGate::NOT_OVER, "gate: no drag, pointer on another window: not over");
    check(startGate(true, true) == eStartGate::YIELD, "gate: a drag is going: yield");
    check(startGate(true, false) == eStartGate::YIELD, "gate: a drag is going wins over the pointer");
    {
        SUnsnap u;
        u.start(300, 200, 800, 600, 0.25, 10, T0);
        u.abort();
        check(!u.active, "gate: abort stops the unsnap where it is");
    }
    // YIELD mid-unsnap: everything stops, and the title press's release reaches the compositor (it ends the other drag)
    {
        SClicks c;
        SPairs  g;
        SUnsnap u;
        bool    pending = true;
        press(c, g, down(eHit::TITLE), true, T0);
        u.start(300, 200, 800, 600, 0.25, 10, T0);
        check(!passGate(eStartGate::YIELD, g, u, pending), "gate: yield does not go on");
        check(!u.active && !pending, "gate: yield stops the unsnap and the delayed start");
        check(same(release(&c, g), eAction::NONE, false), "gate: after yield the release passes to the compositor");
    }
    // NOT_OVER: no drag, the press stays a click (its release is taken)
    {
        SClicks c;
        SPairs  g;
        SUnsnap u;
        bool    pending = true;
        press(c, g, down(eHit::TITLE), true, T0);
        check(!passGate(eStartGate::NOT_OVER, g, u, pending) && !pending, "gate: not over does not go on and drops the delayed start");
        check(same(release(&c, g), eAction::NONE, true), "gate: after not over the release is taken, as a click");
    }
    // GO: nothing touched
    {
        SClicks c;
        SPairs  g;
        SUnsnap u;
        bool    pending = true;
        press(c, g, down(eHit::TITLE), true, T0);
        u.start(300, 200, 800, 600, 0.25, 10, T0);
        check(passGate(eStartGate::GO, g, u, pending) && u.active && pending, "gate: go changes nothing");
        check(same(release(&c, g), eAction::NONE, true), "gate: after go (no drag yet) the release is still the bar's");
    }

    // grab with valid arguments: every way it does not start is a Lua error, each with its cause
    for (int i = 0; i < 8; ++i) {
        const bool found = i & 1, focused = i & 2, started = i & 4;
        const auto E     = grabStartError(found, focused, started);
        check((found && focused && started) == E.empty(), "grab: error unless found, focused and started (case " + std::to_string(i) + ")");
    }
    check(grabStartError(false, false, false) != grabStartError(true, false, false) && grabStartError(true, false, false) != grabStartError(true, true, false),
          "grab: not found, focus refused and start refused say different things");
    check(grabStartError(true, true, false) == grabRefused(), "grab: a refused start keeps its message");
    check(grabStartError(false, true, true).starts_with("grab: "), "grab: errors start with grab:");
#endif

    if (fails)
        std::printf("%d falha(s)\n", fails);
    return fails ? 1 : 0;
}
