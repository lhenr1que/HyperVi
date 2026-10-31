// T4 red baseline: the click decisions of plugin/barDeco.cpp and luaGrab in plugin/main.cpp at bf2049a, extracted by hand
// (mouse path only; the test adapts its per-bar press/release to the alpha-0 SClicks/SPairs split; touch is covered by the structural check). Model of the
// baseline, not the compiled plugin: onMouseButton :154-194, handleDownEvent :348-428, handleUpEvent :612-659,
// onMouseMove :282-325 (pending drag), luaGrab main.cpp :304-322.
#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <linux/input-event-codes.h>

namespace KwIn {
    enum class eHit : uint8_t { OUTSIDE, BUTTON, ICON, TITLE };
    enum class eAction : uint8_t { NONE, BUTTON, ICON_MENU, BAR_MENU, DOUBLE_CLICK, FOCUS };

    struct SPress {
        uint32_t button  = BTN_LEFT;
        bool     pressed = true;
        bool     taken   = false;
        bool     valid   = false;
        eHit     hit     = eHit::OUTSIDE;
    };

    struct SResult {
        eAction action = eAction::NONE;
        bool    cancel = false;
    };

    // host facts the baseline read that the alpha-0 decision no longer needs
    inline bool g_focused  = true; // the bar's window has focus (at release)
    inline bool g_floating = true;
    inline bool g_maximized = false;

    struct SClicks {
        bool dragging = false, pending = false, cancelledDown = false;
        int  dragStarts = 0, dragEnds = 0, snaps = 0; // KwDrag::begin, changeMouseBindMode/mouse 0movewindow, snapOnRelease
        std::optional<std::chrono::steady_clock::time_point> lastTitle;

        SResult up() {
            SResult r;
            if (!g_focused) {
                if (dragging)
                    ++dragEnds;
                dragging = cancelledDown = pending = false;
                return r; // release not cancelled
            }
            if (cancelledDown)
                r.cancel = true;
            cancelledDown = false;
            if (dragging) {
                ++dragEnds;
                dragging = false;
                ++snaps;
            }
            pending = false;
            return r;
        }

        void motion() {
            if (!pending)
                return;
            pending = false;
            ++dragStarts;
            dragging = true;
        }

        SResult onButton(const SPress& e, bool doubleClickOn, std::chrono::steady_clock::time_point now) {
            if (e.button != BTN_LEFT && e.button != BTN_RIGHT)
                return {};
            if (e.pressed && e.taken)
                return {};
            if (!e.pressed && e.button != BTN_RIGHT && (dragging || cancelledDown || pending))
                return up();
            if (!e.valid)
                return {};
            if (!e.pressed) {
                if (e.button != BTN_RIGHT)
                    return up();
                return {}; // right release ignored
            }
            if (e.button == BTN_RIGHT) {
                if (e.hit != eHit::OUTSIDE)
                    return {eAction::BAR_MENU, true};
                return {};
            }
            if (e.hit == eHit::OUTSIDE) {
                if (dragging)
                    ++dragEnds;
                dragging = pending = false;
                return {};
            }
            cancelledDown = true;
            if (e.hit == eHit::BUTTON)
                return {eAction::BUTTON, true};
            if (e.hit == eHit::ICON) {
                pending = false;
                return {eAction::ICON_MENU, true};
            }
            if (doubleClickOn && lastTitle && now - *lastTitle < std::chrono::milliseconds(400)) {
                pending = false;
                return {eAction::DOUBLE_CLICK, true};
            }
            lastTitle = now;
            if (g_floating && !g_maximized) {
                ++dragStarts;
                dragging = true;
            } else
                pending = true;
            return {eAction::FOCUS, true};
        }
    };

    // bf2049a onMouseButton/handleDownEvent after the decision: cancel, focus, raise when floating (left press only), act
    template <class H>
    void apply(const SResult& r, H& host) {
        if (r.cancel)
            host.cancel();
        if (r.action == eAction::NONE)
            return;
        host.focus();
        if (r.action != eAction::BAR_MENU)
            host.raiseIfFloating();
        host.run(r.action);
    }

    // baseline: a valid request starts the grab (no error)
    inline std::string grabError(bool modeIsString, bool addressIsString, std::string_view mode) {
        if (!modeIsString || !addressIsString)
            return "grab: expected (\"move\"|\"resize\", address)";
        if (mode != "move" && mode != "resize")
            return "grab: mode must be move or resize";
        return "";
    }
}
