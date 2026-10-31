// SPDX-License-Identifier: MIT
// Part of kwinish (Copyright (c) 2026 lhen).
// The bar's click decisions, with no compositor: which press a bar takes, what it does with it, and which release goes
// with it. barDeco.cpp feeds it the pointer test and runs the result. Alpha-0 (openspec disable-titlebar-drag-alpha0): no
// press moves or resizes the window; every press a bar takes has its release taken too, wherever the pointer or focus
// went. Tested by tools/test-plugin-input.sh.
#pragma once

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
            if (SLOT < 0 || !e.pressed || e.taken || !e.valid || e.hit == eHit::OUTSIDE)
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

    // hl.plugin.kwinish.grab(mode, address): the Lua error for this call. Bad arguments keep their messages; a valid
    // request is refused too, since the window menu's Move/Resize is off in alpha-0.
    inline std::string grabError(bool modeIsString, bool addressIsString, std::string_view mode) {
        if (!modeIsString || !addressIsString)
            return "grab: expected (\"move\"|\"resize\", address)";
        if (mode != "move" && mode != "resize")
            return "grab: mode must be move or resize";
        return "grab: unavailable in this build (move/resize from the title bar and the window menu are off in alpha-0)";
    }
}
