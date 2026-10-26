// SPDX-License-Identifier: MIT
// Part of kwinish (Copyright (c) 2026 lhen).
#pragma once

#include <string>
#include <cairo.h>

// App icons for the title bar: find the icon file for a window class, render it to a cairo surface.
namespace KwIcon {
    // Best icon file for a window class (desktop entries by StartupWMClass or file name, then hicolor and other
    // themes, then pixmaps), or "". The first call scans the desktop entries once and caches the result.
    std::string      findIconPath(const std::string& windowClass, int wantPx);

    // Renders a png or svg to a size x size ARGB32 surface, keeping the aspect ratio; nullptr on failure. Caller destroys.
    cairo_surface_t* load(const std::string& path, int size);

    // Button glyphs drawn with strokes, so every button gets the same size and weight whatever fonts are installed.
    // name: minimize maximize close pin (anything else gives nullptr). Caller destroys.
    cairo_surface_t* glyph(const std::string& name, int px, double r, double g, double b, double a);
}
