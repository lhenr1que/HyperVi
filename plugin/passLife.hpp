// SPDX-License-Identifier: MIT
// Part of HyperVi (Copyright (c) 2026 lhenr1que).
// Q1 (docs/port-0-57/QUIESCENCE.md): how many of the plugin's pass elements are alive. Their vtables and deleters are code
// of this library, so PLUGIN_EXIT expects 0 (the host destroys queued passes when the render session resets). A member of
// each pass class; render and unload run on the compositor's one thread. Tested by tools/test-plugin-lifecycle.sh.
#pragma once

namespace KwPass {
    inline int g_live = 0;

    struct SLife {
        SLife() {
            ++g_live;
        }
        SLife(const SLife&) {
            ++g_live;
        }
        SLife& operator=(const SLife&) = default;
        ~SLife() {
            --g_live;
        }
    };
}
