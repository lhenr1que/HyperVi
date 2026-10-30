// SPDX-License-Identifier: MIT
// Part of kwinish (Copyright (c) 2026 lhen).
// Tests for actionGate.hpp (no Hyprland): bash tools/test-plugin-actions.sh
#include "actionGate.hpp"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <limits>
#include <optional>
#include <string>

static int fails = 0;

static void check(bool okay, const std::string& what) {
    std::printf("%s - %s\n", okay ? "ok" : "FALHOU", what.c_str());
    fails += okay ? 0 : 1;
}

int main() {
    using namespace std::chrono_literals;
    using clk = std::chrono::steady_clock;

    // K7: the pixel argument the 0.56.2 call built, minus ",activewindow"; the (int) casts stay at the call site
    check(KwAct::exactPixelArg(10, 20) == "exact 10 20", "exact pixel argument");
    check(KwAct::exactPixelArg(-3, 0) == "exact -3 0", "negative coordinate kept");
    check(KwAct::exactPixelArg((int)-3.7, (int)7.9) == "exact -3 7", "casts truncate toward zero, as before");

    // K5: the translator's guard (resizewindowpixel): SIZ.x < 1 || SIZ.y < 1 refuses
    check(KwAct::usableSize(1, 1), "1x1 is usable");
    check(KwAct::usableSize(800, 600), "800x600 is usable");
    check(!KwAct::usableSize(0.999, 600), "width below 1 refused");
    check(!KwAct::usableSize(800, 0), "height 0 refused");
    check(!KwAct::usableSize(-5, -5), "negative refused");
    const double NAN_ = std::numeric_limits<double>::quiet_NaN();
    check(KwAct::usableSize(NAN_, 600), "NaN passes, exactly like the translator's comparison");

    // K12: the 0.56.2 exec dispatcher reported success = PROC.has_value() && PROC.value() > 0
    check(!KwAct::spawnStarted(std::nullopt), "spawn nullopt is a failure");
    check(!KwAct::spawnStarted(std::optional<uint64_t>{0}), "spawn pid 0 is a failure");
    check(KwAct::spawnStarted(std::optional<uint64_t>{4242}), "spawn pid > 0 is a success");

    // failure diagnostics: first failure logged, then at most one line per window per site, with the count skipped
    {
        KwAct::SFailureGate g;
        const auto          T0 = clk::time_point{} + 1000s;
        auto                a  = g.admit(T0);
        check(a.has_value() && *a == 0, "first failure is logged, nothing suppressed");
        check(!g.admit(T0 + 1s).has_value(), "second failure inside the window is suppressed");
        check(!g.admit(T0 + 9s).has_value(), "third failure inside the window is suppressed");
        a = g.admit(T0 + KwAct::FAILURE_LOG_WINDOW);
        check(a.has_value() && *a == 2, "after the window: logged, reports 2 suppressed");
        check(!g.admit(T0 + KwAct::FAILURE_LOG_WINDOW + 1s).has_value(), "window restarts at the logged line");
        a = g.admit(T0 + 3 * KwAct::FAILURE_LOG_WINDOW);
        check(a.has_value() && *a == 1, "suppressed count resets after each logged line");
    }
    {
        // a clock that goes backwards (it cannot for steady_clock, but the gate must not lock up forever)
        KwAct::SFailureGate g;
        const auto          T0 = clk::time_point{} + 1000s;
        (void)g.admit(T0);
        check(!g.admit(T0 - 5s).has_value(), "earlier timestamp inside the window is suppressed");
        check(g.admit(T0 + KwAct::FAILURE_LOG_WINDOW).has_value(), "and the gate still opens after the window");
    }
    {
        // gates are independent per site
        KwAct::SFailureGate a, b;
        const auto          T0 = clk::time_point{} + 1000s;
        (void)a.admit(T0);
        check(b.admit(T0 + 1s).has_value(), "a second site is not suppressed by the first");
    }
    check(KwAct::FAILURE_LOG_WINDOW == 10s, "window is 10 s");

    // site names are fixed, distinct and non-empty
    bool namesOk = true;
    for (size_t i = 0; i < KwAct::SITE_COUNT; ++i) {
        const std::string_view n = KwAct::siteName((KwAct::eSite)i);
        namesOk                  = namesOk && !n.empty() && n != "?";
        for (size_t j = 0; j < i; ++j)
            namesOk = namesOk && n != KwAct::siteName((KwAct::eSite)j);
    }
    check(namesOk, "every site has a distinct name");
    check(KwAct::siteName(KwAct::SITE_COUNT) == "?", "out-of-range site is '?'");

    return fails > 0 ? 1 : 0;
}
