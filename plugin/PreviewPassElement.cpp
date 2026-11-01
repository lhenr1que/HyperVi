// SPDX-License-Identifier: MIT
// Part of HyperVi (Copyright (c) 2026 lhenr1que).
#include "PreviewPassElement.hpp"
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>

using namespace Render::GL;

std::vector<UP<IPassElement>> CPreviewPassElement::draw(Render::CRenderContext& ctx) {
    g_pHyprOpenGL->renderRect(ctx, data.box, data.fill, {.round = data.round});

    // outline: four thin rects (a rect with a hole would need a matte)
    const double b = data.border;
    const CBox   top{data.box.x, data.box.y, data.box.w, b}, bottom{data.box.x, data.box.y + data.box.h - b, data.box.w, b};
    const CBox   left{data.box.x, data.box.y + b, b, data.box.h - 2 * b}, right{data.box.x + data.box.w - b, data.box.y + b, b, data.box.h - 2 * b};
    for (const auto& s : {top, bottom, left, right})
        g_pHyprOpenGL->renderRect(ctx, s, data.line, {});
    return {};
}
