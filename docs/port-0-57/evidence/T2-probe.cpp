// T2 probe (group W) — installed Hyprland 0.57.0 headers, Makefile flags, -fsyntax-only.
// Default: the 0.57 forms used by T2 in plugin/main.cpp compile with the expected types.
// -DPROBE_NEG=1..5: each 0.56.2 form T2 replaces must fail to compile.
#include <hyprland/src/desktop/view/window/Window.hpp>
#include <hyprland/src/desktop/view/window/WindowPresentation.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include "../../../plugin/barDeco.hpp"

#include <span>
#include <type_traits>

using Desktop::View::CWindow;

// main.cpp onNewWindow: X11 "no border" (option A, live), decorations list, registration.
static_assert(std::is_same_v<decltype(std::declval<CWindow&>().backend().traits()), Desktop::View::SBackendTraits>);
static_assert(std::is_same_v<decltype(std::declval<CWindow&>().presentation().decorations()), std::span<const SP<IHyprWindowDecoration>>>);
static_assert(std::is_same_v<decltype(&HyprlandAPI::addWindowDecoration), bool (*)(HANDLE, PHLWINDOW, SP<IHyprWindowDecoration>&&)>);
static_assert(std::is_convertible_v<SP<CHyprBar>, SP<IHyprWindowDecoration>>, "the bar is handed over as a shared pointer");
static_assert(!std::is_convertible_v<UP<CHyprBar>, SP<IHyprWindowDecoration>>, "the 0.56.2 unique pointer no longer binds");
static_assert(std::is_same_v<decltype(std::declval<CHyprBar&>().m_self), WP<CHyprBar>>);
// main.cpp onUpdateWindowRules / pluginInit
static_assert(std::is_same_v<decltype(std::declval<CWindow&>().presentation().updateDecorations()), void>);
static_assert(std::is_same_v<decltype(std::declval<const CWindow&>().mapped()), bool>);

// The exact expressions used in plugin/main.cpp.
[[maybe_unused]] static void t2Forms(PHLWINDOW window, SP<CHyprBar> bar) {
    if (!window->backend().traits().suggestsNoBorder) {
        (void)std::ranges::any_of(window->presentation().decorations(), [](const auto& d) { return d->getDisplayName() == "Hyprbar" || d->getDisplayName() == "KwinishBar"; });
        bar->m_self = bar;
        if (!HyprlandAPI::addWindowDecoration(nullptr, window, SP<IHyprWindowDecoration>(bar)))
            return;
    }
    window->presentation().updateDecorations();
    (void)(window->isHidden() || !window->mapped());
#if PROBE_NEG == 1
    (void)window->m_X11DoesntWantBorders;
#elif PROBE_NEG == 2
    (void)window->m_windowDecorations;
#elif PROBE_NEG == 3
    UP<CHyprBar> old;
    HyprlandAPI::addWindowDecoration(nullptr, window, std::move(old));
#elif PROBE_NEG == 4
    window->updateWindowDecos();
#elif PROBE_NEG == 5
    (void)window->m_isMapped;
#endif
}
