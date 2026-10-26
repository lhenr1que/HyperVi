// SPDX-License-Identifier: MIT
// Part of kwinish (Copyright (c) 2026 lhen).
#include "PreviewPassElement.hpp"
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>

using namespace Render::GL;

std::vector<UP<IPassElement>> CPreviewPassElement::draw() {
    g_pHyprOpenGL->renderRect(data.box, data.fill, {.round = data.round});

    // outline: four thin rects (a rect with a hole would need a matte)
    const double b = data.border;
    const CBox   top{data.box.x, data.box.y, data.box.w, b}, bottom{data.box.x, data.box.y + data.box.h - b, data.box.w, b};
    const CBox   left{data.box.x, data.box.y + b, b, data.box.h - 2 * b}, right{data.box.x + data.box.w - b, data.box.y + b, b, data.box.h - 2 * b};
    for (const auto& s : {top, bottom, left, right})
        g_pHyprOpenGL->renderRect(s, data.line, {});
    return {};
}
