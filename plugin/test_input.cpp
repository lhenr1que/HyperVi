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

// The drag the baseline started or ended; the alpha-0 header has no drag state at all (plugin sources are checked for
// begin/end calls by tools/test-plugin-input.sh).
static int drags(const SClicks& c) {
#ifdef KW_BASELINE
    return c.dragStarts + c.dragEnds + c.snaps + c.dragging + c.pending;
#else
    (void)c;
    return 0;
#endif
}
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

    // D1: a press on the title never starts a drag: floating, tiled, maximized, with or without motion
    for (const auto& [floating, maximized, name] : {std::tuple{true, false, "floating"}, {false, false, "tiled"}, {true, true, "maximized"}}) {
        facts(true, floating, maximized);
        SClicks c;
        SPairs  g;
        const auto P = press(c, g, down(eHit::TITLE), true, T0);
        motion(c);
        const auto R = release(&c, g);
        check(same(P, eAction::FOCUS, true), std::string(name) + ": title press focuses and is taken");
        check(same(R, eAction::NONE, true), std::string(name) + ": its release is taken too");
        check(drags(c) == 0, std::string(name) + ": no drag started, ended or snapped");
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
        check(drags(c) == 0, "icon: no drag");
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
        check(drags(c) == 0, "double click: no drag");
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
    check(grabError(true, true, "move").find("unavailable") != std::string::npos, "grab move is refused as unavailable");
    check(grabError(true, true, "resize").find("unavailable") != std::string::npos, "grab resize is refused as unavailable");

    if (fails)
        std::printf("%d falha(s)\n", fails);
    return fails ? 1 : 0;
}
