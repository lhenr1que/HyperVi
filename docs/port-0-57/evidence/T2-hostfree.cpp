// T2 host-free test: plugin/main.cpp onNewWindow, extracted verbatim by T2-probe-run.sh into
// $T/onNewWindow.inc, against a fake host that keeps the 0.57 contract of addWindowDecoration
// (src/plugins/PluginAPI.cpp:143–157: on false it returns before touching the pointer; on true the window owns it).
// Real hyprutils SP/WP, so ownership and weak-pointer expiry are the real ones.
#include <hyprutils/memory/SharedPtr.hpp>
#include <hyprutils/memory/WeakPtr.hpp>

#include <algorithm>
#include <cstdio>
#include <span>
#include <string>
#include <vector>

using namespace Hyprutils::Memory;
template <class T> using SP = CSharedPointer<T>;
template <class T> using WP = CWeakPointer<T>;

struct IHyprWindowDecoration {
    virtual ~IHyprWindowDecoration() = default;
    virtual std::string getDisplayName() = 0;
};
struct COther : IHyprWindowDecoration {
    std::string n;
    explicit COther(std::string s) : n(std::move(s)) {}
    std::string getDisplayName() override { return n; }
};
struct SBackendTraits { bool suggestsNoBorder : 1 = false; };
struct CBackend { SBackendTraits t; SBackendTraits traits() const { return t; } };
struct CPresentation {
    std::vector<SP<IHyprWindowDecoration>> v;
    std::span<const SP<IHyprWindowDecoration>> decorations() const { return v; }
};
struct CWindow {
    CBackend b; CPresentation p;
    CBackend& backend() { return b; }
    CPresentation& presentation() { return p; }
};
using PHLWINDOW = SP<CWindow>;

struct CHyprBar;
struct SGlobalState { std::vector<WP<CHyprBar>> bars; };
static SGlobalState  gs;
[[maybe_unused]] static SGlobalState* g_pGlobalState = &gs; // unused only when a mutant drops every use
static int           liveBars = 0, selfAtAdd = 0, publishedAtAdd = 0;

struct CHyprBar : IHyprWindowDecoration {
    WP<CHyprBar> m_self;
    explicit CHyprBar(PHLWINDOW) { ++liveBars; }
#ifndef T2_DTOR_NO_ERASE
    ~CHyprBar() override { std::erase(g_pGlobalState->bars, m_self); --liveBars; } // as barDeco.cpp:74–78
#else // variant: the guard alone must keep a refused bar out of bars, without the destructor's rollback
    ~CHyprBar() override { --liveBars; }
#endif
    std::string getDisplayName() override { return "KwinishBar"; }
};

static void* PHANDLE = nullptr;
static bool  hostFails = false;
static int   addCalls = 0;
namespace HyprlandAPI {
    bool addWindowDecoration(void*, PHLWINDOW w, SP<IHyprWindowDecoration>&& d) {
        ++addCalls;
        publishedAtAdd += (int)std::ranges::count_if(gs.bars, [&](const auto& b) { return (void*)b.get() == (void*)d.get(); });
        if (hostFails || !d)
            return false;
        auto* bar = dynamic_cast<CHyprBar*>(d.get());
        if (bar && bar->m_self.lock().get() == bar)
            ++selfAtAdd;
        w->p.v.push_back(std::move(d));
        return true;
    }
}

#include "onNewWindow.inc"

static int fails = 0;
#define CHECK(c, what) do { if (!(c)) { std::printf("FAIL %s: %s\n", name, what); ++fails; } } while (0)
static void reset() { gs.bars.clear(); hostFails = false; addCalls = 0; selfAtAdd = 0; publishedAtAdd = 0; }
static int barsIn(const PHLWINDOW& w) {
    return (int)std::ranges::count_if(w->p.v, [](const auto& d) { return d->getDisplayName() == "KwinishBar"; });
}
// every published weak pointer is alive and is a bar owned by some window decoration list
static bool noDangling() { return std::ranges::all_of(gs.bars, [](const auto& b) { return b && !b.expired(); }); }

int main() {
    { const char* name = "S1 normal window"; reset(); auto w = makeShared<CWindow>();
      onNewWindow(w);
      CHECK(addCalls == 1 && barsIn(w) == 1, "one bar registered");
      CHECK(gs.bars.size() == 1 && gs.bars[0].lock().get() == (CHyprBar*)w->p.v.back().get(), "published once, the registered bar");
      CHECK(selfAtAdd == 1, "m_self set before the host sees the bar");
      CHECK(publishedAtAdd == 0, "published only after the host took it");
      CHECK(liveBars == 1 && noDangling(), "alive, no dangling"); }
#ifndef T2_DTOR_NO_ERASE
    { const char* name = "S1 teardown"; CHECK(liveBars == 0 && gs.bars.empty(), "window gone -> bar gone, unpublished by its destructor"); }
#endif
    { const char* name = "S2 suggestsNoBorder"; reset(); auto w = makeShared<CWindow>(); w->b.t.suggestsNoBorder = true;
      onNewWindow(w);
      CHECK(addCalls == 0 && gs.bars.empty() && liveBars == 0 && w->p.v.empty(), "no bar"); }
    { const char* name = "S3 already has KwinishBar"; reset(); auto w = makeShared<CWindow>(); onNewWindow(w); addCalls = 0;
      onNewWindow(w);
      CHECK(addCalls == 0 && barsIn(w) == 1 && gs.bars.size() == 1 && liveBars == 1, "no second bar"); }
    { const char* name = "S4 already has Hyprbar"; reset(); auto w = makeShared<CWindow>(); w->p.v.push_back(makeShared<COther>("Hyprbar"));
      onNewWindow(w);
      CHECK(addCalls == 0 && gs.bars.empty() && liveBars == 0 && w->p.v.size() == 1, "no bar"); }
    { const char* name = "S5 unrelated decoration"; reset(); auto w = makeShared<CWindow>(); w->p.v.push_back(makeShared<COther>("Border"));
      onNewWindow(w);
      CHECK(addCalls == 1 && barsIn(w) == 1 && gs.bars.size() == 1, "bar added"); }
    { const char* name = "S6 host refuses"; reset(); auto w = makeShared<CWindow>(); hostFails = true;
      onNewWindow(w);
      CHECK(addCalls == 1, "registration attempted");
      CHECK(gs.bars.empty() && publishedAtAdd == 0, "never published, not even while the host decides");
      CHECK(liveBars == 0 && w->p.v.empty(), "bar destroyed, nothing left on the window");
      hostFails = false; onNewWindow(w);
      CHECK(barsIn(w) == 1 && gs.bars.size() == 1 && noDangling(), "a later success registers normally"); }
#ifndef T2_DTOR_NO_ERASE
    { const char* name = "S7 host drops the bar later"; reset(); auto w = makeShared<CWindow>(); onNewWindow(w);
      w->p.v.clear();
      CHECK(gs.bars.empty() && liveBars == 0, "removal unpublishes, no dangling weak pointer"); }
#endif
    std::printf(fails ? "FAILURES %d\n" : "ALL OK\n", fails);
    return fails ? 1 : 0;
}
