// SPDX-License-Identifier: MIT
// Part of kwinish (Copyright (c) 2026 lhen).
// Q1 pass counter (plugin/passLife.hpp), no Hyprland needed. Run by tools/test-plugin-lifecycle.sh.
#include "passLife.hpp"

#include <cstdio>
#include <memory>
#include <utility>
#include <vector>

static int fails = 0;
static void check(bool ok, const char* what) {
    std::printf("%s - %s\n", ok ? "ok" : "FALHOU", what);
    fails += !ok;
}

struct SPass { // stands for CBarPassElement/CPreviewPassElement: the counter is a member
    KwPass::SLife life;
    int           payload = 0;
};

int main() {
    check(KwPass::g_live == 0, "starts at 0");
    {
        SPass a;
        check(KwPass::g_live == 1, "construct counts 1");
        SPass b = a;
        check(KwPass::g_live == 2, "copy counts another");
        SPass c = std::move(b);
        check(KwPass::g_live == 3, "move counts another (the moved-from one still dies)");
        b = c;
        check(KwPass::g_live == 3, "assignment changes nothing");
    }
    check(KwPass::g_live == 0, "back to 0 after scope");
    {
        std::vector<std::unique_ptr<SPass>> queue; // the host's render queue holds UP<IPassElement>
        for (int i = 0; i < 5; i++)
            queue.push_back(std::make_unique<SPass>());
        check(KwPass::g_live == 5, "5 queued");
        queue.erase(queue.begin(), queue.begin() + 2);
        check(KwPass::g_live == 3, "erase 2 leaves 3");
        queue.clear(); // CRenderContext::reset() clears its pass
        check(KwPass::g_live == 0, "clear leaves 0");
    }
    return fails ? 1 : 0;
}
