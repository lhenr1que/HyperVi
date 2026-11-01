// SPDX-License-Identifier: MIT
// Part of HyperVi (Copyright (c) 2026 lhenr1que).
// The bar's click decisions, with no compositor: which press a bar takes, what it does with it, and which release goes
// with it, and the bar's drag. barDeco.cpp feeds it the pointer test and runs the result. Alpha-1 (openspec
// enable-titlebar-drag-alpha1, end policy alt5-rc): every press a bar takes has its release taken too, wherever the pointer
// or focus went, except the press that became a drag, whose release ends it in the compositor; the bar never asks the
// compositor to end a drag and yields to any drag in progress. Tested by tools/test-plugin-input.sh.
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <linux/input-event-codes.h>

namespace KwIn {
    enum class eHit : uint8_t { OUTSIDE, BUTTON, ICON, TITLE }; // where a press lands on the bar
    enum class eAction : uint8_t { NONE, BUTTON, ICON_MENU, BAR_MENU, DOUBLE_CLICK, FOCUS };

    struct SPress {
        uint32_t button  = BTN_LEFT;
        bool     pressed = true;
        bool     taken   = false; // a press another bar (overlapping) already took
        bool     valid   = false; // the bar takes input here (inputIsValid); only read for presses
        eHit     hit     = eHit::OUTSIDE;
        bool     dragActive = false; // a drag (anyone's) is in progress: the bar yields
    };

    struct SResult {
        eAction action = eAction::NONE;
        bool    cancel = false; // the event is the bar's: it does not reach the app
    };

    inline constexpr auto DOUBLE_CLICK_WINDOW = std::chrono::milliseconds(400);

    // Which taken presses still wait for their release. One for all bars (g_pGlobalState->pairs), so a release still
    // finds its press when the bar that took it is gone (the close button); main.cpp's button listener takes releases.
    struct SPairs {
        // shortcut: pairs by button only (IPointer::SButtonEvent in 0.57 carries no pointer), so two pointers holding the same
        // button can swap releases; accepted by the owner on 31/10/2026, revisit if the host event gains a device
        bool held[2] = {false, false}; // left, right

        static int slot(uint32_t button) {
            return button == BTN_LEFT ? 0 : button == BTN_RIGHT ? 1 : -1;
        }
        // the title press became a drag: its release must reach the compositor, which ends the drag with it
        void dragStarted(uint32_t button) {
            if (const int S = slot(button); S >= 0)
                held[S] = false;
        }
        SResult onRelease(uint32_t button) {
            const int S = slot(button);
            if (S < 0 || !held[S])
                return {};
            held[S] = false;
            return {eAction::NONE, true};
        }
    };

    // One per bar: decides presses (releases go to SPairs::onRelease).
    struct SClicks {
        std::optional<std::chrono::steady_clock::time_point> lastTitle; // last plain press on the title

        SResult onPress(const SPress& e, SPairs& pairs, bool doubleClickOn, std::chrono::steady_clock::time_point now) {
            // middle, side and extra buttons are not ours (they used to count as a left click)
            const int SLOT = SPairs::slot(e.button);
            if (SLOT < 0 || !e.pressed || e.taken || !e.valid || e.hit == eHit::OUTSIDE || e.dragActive)
                return {};
            pairs.held[SLOT] = true;

            if (SLOT == 1)
                return {eAction::BAR_MENU, true};
            if (e.hit == eHit::BUTTON)
                return {eAction::BUTTON, true};
            if (e.hit == eHit::ICON)
                return {eAction::ICON_MENU, true};
            if (doubleClickOn && lastTitle && now - *lastTitle < DOUBLE_CLICK_WINDOW)
                return {eAction::DOUBLE_CLICK, true}; // lastTitle kept, as before: a third quick click is another double
            lastTitle = now;
            return {eAction::FOCUS, true};
        }
    };

    // What the bar's drag does next (alt5-rc: the bar never asks the compositor to end a drag).
    enum class eDragStep : uint8_t { NONE, WAIT, SNAP, CLEAR, KEEP, RESTART };

    struct SDrag {
        bool active   = false; // this bar started the drag and its button is still down
        bool released = false; // its release went by: the compositor ends the drag with it
        bool rearm    = false; // Shift changed mid-drag

        void start(bool started) {
            active   = started;
            released = false;
            rearm    = false;
        }
        // the bar's button went up: the compositor ends a drag still going with this release (SNAP comes from onEnded)
        eDragStep onRelease(bool dragGoing) {
            if (!active)
                return eDragStep::NONE;
            rearm = false;
            if (dragGoing) {
                released = true;
                return eDragStep::WAIT;
            }
            active = false;
            return eDragStep::CLEAR;
        }
        // the compositor ended a drag: snap if it was the bar's release, keep it for the rearm after Shift, else let go
        eDragStep onEnded() {
            if (!active)
                return eDragStep::NONE;
            if (released) {
                active = released = false;
                return eDragStep::SNAP;
            }
            if (rearm)
                return eDragStep::KEEP;
            active = false;
            return eDragStep::CLEAR;
        }
        void onShift() {
            if (active)
                rearm = true;
        }
        // rearm only starts again: never while a drag goes on, never off the window or before an unsnap
        eDragStep onMotion(bool dragGoing, bool overWindow, bool unsnapPending) {
            if (!active || !rearm || dragGoing || !overWindow || unsnapPending)
                return eDragStep::NONE;
            rearm = false;
            return eDragStep::RESTART;
        }
    };

    // Leaving a snap by dragging (KWin): the size eases back to the one before the snap over 140 ms, the window kept under the
    // pointer at the same fraction of its bar, before the compositor's drag starts (alt5-rc: nothing to pause or end).
    struct SUnsnapStep {
        double w = 0, h = 0, x = 0, y = 0; // size and top-left position to apply
        bool   done = false;               // the ease is over: start the drag
    };

    struct SUnsnap {
        inline static constexpr auto DURATION = std::chrono::milliseconds(140);
        bool                                  active = false;
        double                                fromW = 0, fromH = 0, toW = 0, toH = 0, ratio = 0, grabY = 0;
        std::chrono::steady_clock::time_point t0;

        void start(double fromW_, double fromH_, double toW_, double toH_, double ratio_, double grabY_, std::chrono::steady_clock::time_point now) {
            fromW = fromW_, fromH = fromH_, toW = toW_, toH = toH_, ratio = ratio_, grabY = grabY_, t0 = now;
            active = true;
        }
        SUnsnapStep step(double cx, double cy, std::chrono::steady_clock::time_point now) {
            const double P = std::min(1.0, std::chrono::duration<double>(now - t0) / std::chrono::duration<double>(DURATION));
            return at(cx, cy, P);
        }
        // released mid-ease: land on the final size, no drag
        SUnsnapStep finish(double cx, double cy) {
            return at(cx, cy, 1.0);
        }
        // another drag started mid-ease: stop where it is, touch the window no more
        void abort() {
            active = false;
        }

      private:
        SUnsnapStep at(double cx, double cy, double P) {
            const double E = 1.0 - (1.0 - P) * (1.0 - P) * (1.0 - P); // ease-out cubic, as 0.56.2
            SUnsnapStep  r;
            r.w    = fromW + (toW - fromW) * E;
            r.h    = fromH + (toH - fromH) * E;
            r.x    = cx - ratio * r.w;
            r.y    = cy - grabY;
            r.done = P >= 1.0;
            if (r.done)
                active = false;
            return r;
        }
    };

    // Before the bar changes a window's geometry or focus on the way to its drag (openspec guard-titlebar-drag-start): go on,
    // YIELD to a drag already going (anyone's), or NOT_OVER when the pointer left the bar's window (the press stays a click).
    enum class eStartGate : uint8_t { GO, YIELD, NOT_OVER };
    inline eStartGate startGate(bool dragActive, bool pointerOnOwnWindow) {
        if (dragActive)
            return eStartGate::YIELD;
        return pointerOnOwnWindow ? eStartGate::GO : eStartGate::NOT_OVER;
    }
    // What the bar does with the gate; true: go on. YIELD lets the press's release reach the compositor, which ends that drag.
    inline bool passGate(eStartGate g, SPairs& pairs, SUnsnap& unsnap, bool& pending) {
        if (g == eStartGate::GO)
            return true;
        unsnap.abort();
        pending = false;
        if (g == eStartGate::YIELD)
            pairs.dragStarted(BTN_LEFT);
        return false;
    }

    // What onMouseButton does with a decision. H: cancel(), focus(), raiseIfFloating(), run(eAction).
    template <class H>
    void apply(const SResult& r, H& host) {
        if (r.cancel)
            host.cancel();
        if (r.action == eAction::NONE)
            return;
        host.focus();
        if (r.action != eAction::BAR_MENU) // a right click focuses without raising, as before
            host.raiseIfFloating();
        host.run(r.action);
    }

    // the Lua error when the compositor did not start the drag a valid grab asked for
    inline std::string grabRefused() {
        return "grab: the compositor did not start it (a drag is already going, or no window under the pointer)";
    }

    // grab with valid arguments: the Lua error when no drag started (window not found, focus refused, start refused), or ""
    inline std::string grabStartError(bool found, bool focused, bool started) {
        if (!found)
            return "grab: no such window (closed, hidden or not mapped)";
        if (!focused)
            return "grab: the window could not take focus (a layer surface holds the keyboard)";
        return started ? "" : grabRefused();
    }

    // hl.plugin.kwinish.grab(mode, address): the Lua error for bad arguments, or "" when the call may go ahead.
    inline std::string grabError(bool modeIsString, bool addressIsString, std::string_view mode) {
        if (!modeIsString || !addressIsString)
            return "grab: expected (\"move\"|\"resize\", address)";
        if (mode != "move" && mode != "resize")
            return "grab: mode must be move or resize";
        return "";
    }
}
