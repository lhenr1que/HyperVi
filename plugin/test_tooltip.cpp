// SPDX-License-Identifier: MIT
// Part of kwinish (Copyright (c) 2026 lhen).
// Tests for tooltip.hpp (no Hyprland): bash tools/test-plugin-tooltip.sh
#include "tooltip.hpp"

#include <cstdio>
#include <string>

static int fails = 0;

static void check(bool okay, const std::string& what) {
    std::printf("%s - %s\n", okay ? "ok" : "FALHOU", what.c_str());
    fails += okay ? 0 : 1;
}

static bool same(const KwTip::SRect& a, const KwTip::SRect& b) {
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

int main() {
    using KwTip::SRect;

    // locale: the first set, non-empty one of LC_ALL, LC_MESSAGES, LANG
    check(KwTip::portuguese(nullptr, nullptr, "pt_BR.UTF-8"), "LANG=pt_BR is Portuguese");
    check(KwTip::portuguese("", nullptr, "pt_BR.UTF-8"), "an empty LC_ALL is skipped");
    check(!KwTip::portuguese("en_US.UTF-8", nullptr, "pt_BR.UTF-8"), "LC_ALL wins over LANG");
    check(KwTip::portuguese(nullptr, "pt_PT.UTF-8", "en_US.UTF-8"), "LC_MESSAGES wins over LANG");
    check(!KwTip::portuguese(nullptr, nullptr, "C"), "LANG=C is English");
    check(!KwTip::portuguese(nullptr, nullptr, nullptr), "no locale is English");

    // text by action; the flag is the button state (maximized for maximize, pinned for pin)
    check(KwTip::text("minimize", false, true) == "Minimizar", "minimize pt");
    check(KwTip::text("minimize", false, false) == "Minimize", "minimize en");
    check(KwTip::text("maximize", false, true) == "Maximizar", "maximize pt");
    check(KwTip::text("maximize", false, false) == "Maximize", "maximize en");
    check(KwTip::text("maximize", true, true) == "Restaurar", "maximize on a maximized window pt");
    check(KwTip::text("maximize", true, false) == "Restore", "maximize on a maximized window en");
    check(KwTip::text("close", true, true) == "Fechar", "close pt");
    check(KwTip::text("close", false, false) == "Close", "close en");
    check(KwTip::text("pin", false, true) == "Em todas as áreas de trabalho", "pin pt");
    check(KwTip::text("pin", false, false) == "On all workspaces", "pin en");
    check(KwTip::text("pin", true, true) == "Só nesta área de trabalho", "pin on a pinned window pt");
    check(KwTip::text("pin", true, false) == "Only on this workspace", "pin on a pinned window en");
    check(KwTip::text("menu", false, true) == "Menu da janela", "menu (the app icon) pt");
    check(KwTip::text("menu", false, false) == "Window menu", "menu (the app icon) en");
    check(KwTip::text("float", false, true).empty(), "float has no tip");
    check(KwTip::text("fullscreen", false, true).empty(), "fullscreen has no tip");
    check(KwTip::text("exec:kitty", false, false).empty(), "exec: has no tip");

    // placement: centred under the button, 4 px below the bar, inside the monitor
    const SRect MON{0, 0, 1920, 1080};
    check(same(KwTip::place({860, 105, 18, 18}, {100, 100, 800, 28}, 80, 24, MON), {829, 132, 80, 24}), "under the button, centred");
    check(same(KwTip::place({1900, 5, 18, 18}, {1000, 0, 920, 28}, 80, 24, MON), {1840, 32, 80, 24}), "pushed left at the right edge");
    check(same(KwTip::place({1921, 5, 18, 18}, {1920, 0, 600, 28}, 80, 24, {1920, 0, 1920, 1080}), {1920, 32, 80, 24}),
          "pushed right at the left edge of a second monitor");
    check(same(KwTip::place({860, 1055, 18, 18}, {100, 1050, 800, 28}, 80, 24, MON), {829, 1022, 80, 24}), "above the bar when there is no room below");
    check(same(KwTip::place({10, 5, 18, 18}, {0, 0, 100, 28}, 300, 24, {0, 0, 200, 40}), {0, 0, 300, 24}), "wider than the monitor: starts at its left edge, top kept");

    return fails > 0 ? 1 : 0;
}
