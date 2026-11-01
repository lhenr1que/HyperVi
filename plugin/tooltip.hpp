// SPDX-License-Identifier: MIT
// Part of HyperVi (Copyright (c) 2026 lhenr1que).
// The title bar button tooltip, the part that needs no compositor: what it says and where it goes. Tested by
// tools/test-plugin-tooltip.sh.
#pragma once

#include <algorithm>
#include <string>
#include <string_view>

namespace KwTip {
    // Same rule as kwinish-ui: the first set, non-empty one of LC_ALL, LC_MESSAGES, LANG starting with "pt".
    inline bool portuguese(const char* lcAll, const char* lcMessages, const char* lang) {
        for (const char* v : {lcAll, lcMessages, lang})
            if (v && *v)
                return std::string_view(v).starts_with("pt");
        return false;
    }

    // The tip for a button action ("menu" also for the app icon), "" for an action that has none. checked is the button's
    // state (maximized for maximize, pinned for pin): the tip says what the click does, as KDE's "Restore" and "On one desktop".
    inline std::string text(std::string_view action, bool checked, bool pt) {
        if (action == "minimize")
            return pt ? "Minimizar" : "Minimize";
        if (action == "maximize")
            return checked ? (pt ? "Restaurar" : "Restore") : (pt ? "Maximizar" : "Maximize");
        if (action == "close")
            return pt ? "Fechar" : "Close";
        if (action == "pin")
            return checked ? (pt ? "Só nesta área de trabalho" : "Only on this workspace") : (pt ? "Em todas as áreas de trabalho" : "On all workspaces");
        if (action == "menu")
            return pt ? "Menu da janela" : "Window menu";
        return "";
    }

    struct SRect {
        double x = 0, y = 0, w = 0, h = 0;
    };

    // A w x h tip for a button on a bar, global layout coordinates: centred under the button, gap below the bar, kept
    // inside the monitor; above the bar when it does not fit below.
    inline SRect place(const SRect& button, const SRect& bar, double w, double h, const SRect& monitor, double gap = 4) {
        double x = std::max(monitor.x, std::min(button.x + button.w / 2 - w / 2, monitor.x + monitor.w - w));
        double y = bar.y + bar.h + gap;
        if (y + h > monitor.y + monitor.h)
            y = std::max(monitor.y, bar.y - gap - h);
        return {x, y, w, h};
    }
}
