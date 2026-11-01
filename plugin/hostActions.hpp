// SPDX-License-Identifier: MIT
// Part of HyperVi (Copyright (c) 2026 lhenr1que).
// The bar's host actions on Hyprland 0.57, replacing the 0.56.2 legacy dispatchers (g_pKeybindManager->m_dispatchers).
// Each wrapper keeps what the dispatcher translator did around the Config::Actions call (window selection, argument
// parsing, guards) and inspects the result: a failure is logged through a per-site gate (actionGate.hpp), never more
// than one line per site every FAILURE_LOG_WINDOW. A failure never changes what the caller does next, as before.
// Map and differences: docs/port-0-57/P8-DISPATCHERS.md.
#pragma once

#include "actionGate.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/config/shared/actions/ConfigActions.hpp>
#include <hyprland/src/config/supplementary/executor/Executor.hpp>
#include <hyprland/src/debug/log/Logger.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/view/window/Window.hpp>

#include <array>
#include <chrono>
#include <format>
#include <string>

namespace KwAct {
    inline std::array<SFailureGate, SITE_COUNT> g_failureGates{};

    // true if the action succeeded. A failure is logged at the level the host gave it (SILENT: not logged).
    inline bool check(eSite site, const Config::Actions::ActionResult& result) {
        if (result)
            return true;
        const auto& E = result.error();
        if (E.level == Config::eConfigErrorLevel::SILENT || site >= SITE_COUNT)
            return false;
        const auto SKIPPED = g_failureGates[site].admit(std::chrono::steady_clock::now());
        if (!SKIPPED)
            return false;
        const auto LEVEL = E.level == Config::eConfigErrorLevel::ERROR ? Log::ERR : E.level == Config::eConfigErrorLevel::WARNING ? Log::WARN : Log::DEBUG;
        if (*SKIPPED == 0)
            Log::logger->log(LEVEL, "kwinish-bars", "{} failed: {} ({})", siteName(site), E.message, Config::toString(E.code));
        else
            Log::logger->log(LEVEL, "kwinish-bars", "{} failed: {} ({}); {} earlier failure(s) not logged", siteName(site), E.message, Config::toString(E.code), *SKIPPED);
        return false;
    }

    // 0.56.2 resizewindowpixel "<cmd>,activewindow": the focused window (not necessarily the bar's), the size parsed
    // relative to its goal size, refused below 1x1.
    inline bool resizeFocused(eSite site, const std::string& cmd) {
        const auto W = Desktop::focusState()->window();
        if (!W)
            return check(site, Config::Actions::actionError("resizeWindow: no window", Config::eConfigErrorLevel::WARNING, Config::eConfigErrorCode::NO_TARGET));
        const auto SIZ = g_pCompositor->parseWindowVectorArgsRelative(cmd, W->size(Desktop::View::IGeometric::GEOMETRIC_GOAL));
        if (!usableSize(SIZ.x, SIZ.y))
            return check(site, Config::Actions::actionError("Invalid size", Config::eConfigErrorLevel::ERROR, Config::eConfigErrorCode::INVALID_ARGUMENT));
        return check(site, Config::Actions::resize(SIZ, false, W));
    }

    // 0.56.2 movewindowpixel "<cmd>,activewindow": the focused window, the position parsed relative to its goal position.
    inline bool moveFocused(eSite site, const std::string& cmd) {
        const auto W = Desktop::focusState()->window();
        if (!W)
            return check(site, Config::Actions::actionError("moveWindow: no window", Config::eConfigErrorLevel::WARNING, Config::eConfigErrorCode::NO_TARGET));
        const auto POS = g_pCompositor->parseWindowVectorArgsRelative(cmd, W->position(Desktop::View::IGeometric::GEOMETRIC_GOAL));
        return check(site, Config::Actions::move(POS, false, W));
    }

    // 0.56.2 exec: the executor's spawn, a failure being no pid or pid 0 (same messages as the old dispatcher).
    inline bool spawn(eSite site, const std::string& cmd) {
        const auto PROC = Config::Supplementary::executor()->spawn(cmd);
        if (spawnStarted(PROC))
            return true;
        return check(site,
                     Config::Actions::actionError(PROC ? std::format("Failed to start process {}", cmd) : std::format("Failed to start process. No closing bracket in exec rule. {}", cmd),
                                                  Config::eConfigErrorLevel::ERROR, Config::eConfigErrorCode::EXECUTION_FAILED));
    }
}
