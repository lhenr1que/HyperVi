// SPDX-License-Identifier: MIT
// Part of kwinish (Copyright (c) 2026 lhen).
// The part of the bar's host actions that needs no compositor: the arguments and guards the 0.56.2 dispatcher
// translator applied, and the gate that keeps action-failure diagnostics from flooding the log. The Hyprland side
// is hostActions.hpp. Tested by tools/test-plugin-actions.sh.
#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>

namespace KwAct {
    // Where a host action is called from; each site has its own failure-log gate.
    enum eSite : uint8_t {
        SITE_TOUCH_FLOAT = 0, // K4  touch drag start: float the focused window
        SITE_TOUCH_RESIZE,    // K5  touch drag start: exact 50% 50%
        SITE_TOUCH_PIN,       // K6  touch drag start: pin toggle
        SITE_TOUCH_MOVE,      // K7  touch drag: move to the finger
        SITE_TOUCH_TILE,      // K8  touch drag left the bar: tile again
        SITE_MAXIMIZE,        // K10 maximize button
        SITE_FULLSCREEN,      // K10 fullscreen button
        SITE_MINIMIZE,        // K11 minimize button
        SITE_EXEC_BUTTON,     // K12 exec:<cmd> button
        SITE_MENU,            // K12 window menu (kwinish-ui)
        SITE_COUNT,
    };

    inline std::string_view siteName(eSite s) {
        static constexpr std::array<std::string_view, SITE_COUNT> NAMES = {
            "touch float", "touch resize", "touch pin", "touch move", "touch tile", "maximize", "fullscreen", "minimize", "exec button", "window menu",
        };
        return s < SITE_COUNT ? NAMES[s] : "?";
    }

    // "exact X Y": the movewindowpixel argument the 0.56.2 call built, without the ",activewindow" selector.
    // The caller keeps its (int) casts so fractional coordinates are truncated exactly as before.
    inline std::string exactPixelArg(int x, int y) {
        return std::format("exact {} {}", x, y);
    }

    // The resizewindowpixel guard of the 0.56.2 translator, verbatim: refuse if SIZ.x < 1 || SIZ.y < 1 (NaN passes).
    inline bool usableSize(double x, double y) {
        return !(x < 1 || y < 1);
    }

    // The 0.56.2 exec dispatcher reported success as PROC.has_value() && PROC.value() > 0.
    inline bool spawnStarted(const std::optional<uint64_t>& pid) {
        return pid.has_value() && *pid > 0;
    }

    inline constexpr std::chrono::steady_clock::duration FAILURE_LOG_WINDOW = std::chrono::seconds(10);

    // One per call site. The first failure is logged; later ones inside FAILURE_LOG_WINDOW of the last logged line are
    // only counted, and the next logged line reports how many were skipped. Trivially destructible and holds no host
    // objects; whether the module itself can be unloaded safely is the separate unload work (Q4), not shown here.
    struct SFailureGate {
        std::optional<std::chrono::steady_clock::time_point> last;
        uint64_t                                             suppressed = 0;

        // nullopt: do not log this one. Otherwise log it, together with the returned number of skipped failures.
        std::optional<uint64_t> admit(std::chrono::steady_clock::time_point now) {
            if (last && now < *last + FAILURE_LOG_WINDOW) {
                ++suppressed;
                return std::nullopt;
            }
            const uint64_t SKIPPED = suppressed;
            last                   = now;
            suppressed             = 0;
            return SKIPPED;
        }
    };
}
