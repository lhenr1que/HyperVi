// SPDX-License-Identifier: MIT
// Part of HyperVi (Copyright (c) 2026 lhenr1que).
// Starting a pointer drag (move/resize) on Hyprland 0.57, where CKeybindManager::changeMouseBindMode is gone. This is
// the START half of the 0.56.2 body (O:managers/KeybindManager.cpp:949-968) on the 0.57 primitives, nothing more:
//   - a drag already in progress (anyone's) is left alone: nothing is ended, nothing is started;
//   - the window is the one under the pointer (as before; for the menu's resize that is the 0.56.2 behaviour too);
//   - a non-fullscreen MOVE asks the window's decorations first (INPUT_TYPE_DRAG_START), as before;
//   - beginDragTarget keeps its defaults (no forced edge, exclusiveDeviceGrab = false), as before.
// Ending a drag is NOT here: without knowing whose drag is active (session ownership, Q4), ending would also end drags
// started by other components. See docs/port-0-57/P8-DRAG.md.
#pragma once

#include <hyprland/src/desktop/state/ViewHitTester.hpp>
#include <hyprland/src/desktop/state/ViewState.hpp>
#include <hyprland/src/desktop/view/window/Window.hpp>
#include <hyprland/src/desktop/view/window/WindowPresentation.hpp>
#include <hyprland/src/layout/LayoutManager.hpp>
#include <hyprland/src/layout/supplementary/DragController.hpp>
#include <hyprland/src/layout/target/Target.hpp>
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>

#include <cstdint>

namespace KwDrag {
    enum eBeginResult : uint8_t {
        BEGIN_STARTED = 0,      // beginDragTarget was called (the host may still refuse inside it)
        BEGIN_ALREADY_DRAGGING, // a drag was in progress: left untouched
        BEGIN_NO_WINDOW,        // nothing under the pointer (0.56.2: passEvent)
        BEGIN_REFUSED_BY_DECO,  // a decoration took the drag start (0.56.2: passEvent = false)
        BEGIN_BAD_MODE,         // not MBIND_MOVE / MBIND_RESIZE: nothing done
    };

    // MODE must be MBIND_MOVE or MBIND_RESIZE; anything else does nothing (0.56.2 ended the drag for MBIND_INVALID,
    // which this helper deliberately does not do).
    inline eBeginResult begin(eMouseBindMode MODE) {
        if (MODE != MBIND_MOVE && MODE != MBIND_RESIZE)
            return BEGIN_BAD_MODE;
        if (g_layoutManager->dragController()->target())
            return BEGIN_ALREADY_DRAGGING;

        const auto      MOUSECOORDS = g_pInputManager->getMouseCoordsInternal();
        const PHLWINDOW PWINDOW =
            Desktop::viewState()->hitTest().windowAt(MOUSECOORDS, Desktop::View::RESERVED_EXTENTS | Desktop::View::INPUT_EXTENTS | Desktop::View::ALLOW_FLOATING);

        if (!PWINDOW)
            return BEGIN_NO_WINDOW;

        if (!Fullscreen::controller()->isFullscreen(PWINDOW) && MODE == MBIND_MOVE) {
            if (PWINDOW->presentation().checkInputOnDecorations(INPUT_TYPE_DRAG_START, MOUSECOORDS))
                return BEGIN_REFUSED_BY_DECO;
            // H1 (T9): a decoration may start a drag itself and still return false; never replace it
            if (g_layoutManager->dragController()->target())
                return BEGIN_ALREADY_DRAGGING;
        }

        g_layoutManager->beginDragTarget(PWINDOW->layoutTarget(), MODE);
        return BEGIN_STARTED;
    }
}
