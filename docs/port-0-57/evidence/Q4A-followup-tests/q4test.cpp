// Q4-A scenarios. Build: g++/clang++ -std=c++2b -fsanitize=address,undefined q4test.cpp $(pkg-config --cflags --libs hyprutils)
#include "q4model.hpp"
#include "review_cases.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static int fails = 0, oks = 0;
static void check(bool ok, const std::string& what) {
    std::printf("%s - %s\n", ok ? "ok" : "FALHOU", what.c_str());
    (ok ? oks : fails)++;
}

// Variant with a plugin-lifetime listener (P7 §5 item 6): the generation counter is what invalidates ownership.
struct CPersistent {
    uint64_t                generation = 0;
    std::optional<uint64_t> ownedAt;
    WPT                     ownedTarget;
    CHyprSignalListener     listener;
    void                    init() { listener = g_dc->m_events.ended.listen([this] { ++generation; }); }
    bool owns() const { return ownedAt && *ownedAt == generation && !ownedTarget.expired() && g_dc->target() == ownedTarget.lock(); }
    eBeginResult begin(SPT t, eMode m) {
        if (g_dc->target()) return ALREADY;
        const auto G0 = generation;
        g_dc->dragBegin(t, m);
        if (generation != G0 || g_dc->target() != t) return REFUSED;
        ownedAt = G0; ownedTarget = t;
        return STARTED;
    }
    bool endIfOurs() { if (!owns()) return false; ownedAt.reset(); return g_dc->dragEnd(); }
};

static SPT mk(const char* n) { auto t = makeShared<STarget>(); t->name = n; return t; }

// Run one scenario for a policy P. Returns {thirdPartyDragEndedByPlugin, ownDragNotEnded}
template <typename P>
static void scenarios(const char* NAME, bool expectSafe) {
    auto run = [&](const char* sc, auto body) {
        CDragModel dc; g_dc = &dc; g_log.clear();
        P p;
        if constexpr (requires { p.init(); }) p.init();
        if constexpr (requires { p.kind; }) p.kind = std::strstr(NAME, "unconditional") ? 0 : std::strstr(NAME, "target+mode") ? 2 : 1;
        body(dc, p, sc);
    };
    // S1 liveness: our drag, we end it
    run("S1", [&](CDragModel& dc, P& p, const char* sc) {
        auto A = mk("A");
        const auto R = p.begin(A, M_MOVE);
        const bool ended = p.endIfOurs();
        check(R == STARTED && ended && !dc.target(), std::string(NAME) + " " + sc + ": own drag started and ended by the plugin (liveness)");
    });
    // S2 the counterexample: host ends ours (key), user starts on the SAME window, plugin release
    run("S2", [&](CDragModel& dc, P& p, const char* sc) {
        auto A = mk("A");
        p.begin(A, M_MOVE);
        dc.dragEnd();                      // host: key press (U:keybinds/Manager.cpp:223)
        dc.dragBegin(A, M_MOVE);           // user's bind drag on the same window/target, same mode
        const bool ended = p.endIfOurs();
        check(expectSafe ? (!ended && dc.target() == A) : (ended && !dc.target()),
              std::string(NAME) + " " + sc + (expectSafe ? ": user drag on the SAME window kept" : ": user drag on the same window ENDED (counterexample reproduced)"));
    });
    if (!expectSafe) return;
    // S3 begin refused because a third-party drag is active; plugin end must not touch it
    run("S3", [&](CDragModel& dc, P& p, const char* sc) {
        auto A = mk("A"), X = mk("X");
        dc.dragBegin(X, M_MOVE);
        const auto R = p.begin(A, M_MOVE);
        check(R == ALREADY && !p.endIfOurs() && dc.target() == X, std::string(NAME) + " " + sc + ": third-party drag active -> no ownership, untouched");
    });
    // S4 refusal inside dragBegin (unmapped / no workspace / fullscreen workspace) -> `ended` inside begin -> no ownership
    for (int v = 0; v < 4; ++v)
        run("S4", [&](CDragModel& dc, P& p, const char* sc) {
            auto A = mk("A");
            if (v == 3) { A->mapped = false; A->hasWindow = false; } // refusal that emits `ended` WITHOUT resetting the target (U:262-271)
            else (v == 0 ? A->mapped : v == 1 ? A->hasWorkspace : A->fsWorkspaceReject) = (v == 2);
            const auto R = p.begin(A, M_MOVE);
            auto X = mk("X");
            dc.dragBegin(X, M_MOVE); // later third-party drag
            check(R == REFUSED && !p.endIfOurs() && dc.target() == X, std::string(NAME) + " " + sc + "." + std::to_string(v) + ": refused inside dragBegin -> no ownership; later third-party drag kept");
        });
    // S5 our target dies WITHOUT `ended` (dragEnd returns early on an expired target), then a third-party drag
    run("S5", [&](CDragModel& dc, P& p, const char* sc) {
        SPT A = mk("A");
        p.begin(A, M_MOVE);
        A = nullptr;                       // target destroyed; controller's WP expires silently
        dc.dragEnd();                      // host: nothing to end -> no `ended`
        auto X = mk("X");
        dc.dragBegin(X, M_MOVE);
        check(!p.endIfOurs() && dc.target() == X, std::string(NAME) + " " + sc + ": our target died silently (no ended) -> third-party drag kept");
    });
    // S7 reentrancy: inside the host's dragEnd of OUR session a callback starts a third-party drag before `ended`
    run("S7", [&](CDragModel& dc, P& p, const char* sc) {
        auto A = mk("A"), X = mk("X");
        p.begin(A, M_MOVE);
        dc.onFocusDuringEnd = [&] { dc.onFocusDuringEnd = nullptr; dc.dragBegin(X, M_MOVE); }; // window.active listener of someone else
        dc.dragEnd();                      // host ends ours; X starts reentrantly; then `ended` (for A) is emitted
        check(!p.endIfOurs() && dc.target() == X, std::string(NAME) + " " + sc + ": reentrant third-party start during host dragEnd -> kept");
    });
    // S8 reentrancy during OUR OWN end: a callback tries to start a plugin drag
    run("S8", [&](CDragModel& dc, P& p, const char* sc) {
        auto A = mk("A"), B = mk("B");
        p.begin(A, M_MOVE);
        eBeginResult inner = ALREADY;
        dc.onFocusDuringEnd = [&] { dc.onFocusDuringEnd = nullptr; inner = p.begin(B, M_MOVE); };
        p.endIfOurs();
        const bool ownsAfter = p.owns();
        if constexpr (requires { p.inOwnEnd; })
            check(inner == REFUSED && !dc.target() && !ownsAfter, std::string(NAME) + " " + sc + ": plugin start refused while inside its own end (no orphan session)");
        else
            check(!ownsAfter, std::string(NAME) + " " + sc + ": reentrant own start: ownership not kept (conservative; started=" + std::to_string(inner == STARTED) + ")");
    });
    // S9 DRAG_END decoration early return path still emits `ended`
    run("S9", [&](CDragModel& dc, P& p, const char* sc) {
        auto A = mk("A");
        A->decoTakesDragEnd = true;
        p.begin(A, M_MOVE);
        dc.dragEnd();
        dc.dragBegin(A, M_MOVE);
        check(!p.endIfOurs() && dc.target() == A, std::string(NAME) + " " + sc + ": decoration-early-return end still invalidates; later user drag kept");
    });
    // S10 target without window: dragEnd emits `ended` but does NOT reset m_target (U:262-271)
    run("S10", [&](CDragModel& dc, P& p, const char* sc) {
        auto A = mk("A");
        p.begin(A, M_MOVE);
        A->mapped = false; A->hasWindow = false;
        dc.dragEnd();                     // ended emitted, target still set
        check(!p.endIfOurs() && dc.target() == A, std::string(NAME) + " " + sc + ": `ended` without target reset -> ownership dropped (conservative)");
    });
}

int main() {
    std::printf("== naive policies (no ownership): the counterexample\n");
    scenarios<CNaive>("unconditional", false);
    scenarios<CNaive>("target equality", false);
    scenarios<CNaive>("target+mode", false);
    std::printf("== proposed: session-scoped listener + generation + target WP\n");
    scenarios<COwnership>("proposed", true);
    std::printf("== variant: plugin-lifetime listener + generation\n");
    scenarios<CPersistent>("persistent", true);

    // ---- premise A3: an UNGUARDED third-party beginDragTarget while ours is active, on the SAME target -------------
    {
        CDragModel dc; g_dc = &dc; COwnership p; auto A = mk("A");
        p.begin(A, M_MOVE);
        dc.dragBegin(A, M_RESIZE);         // replaces m_target without `ended` (U:160 has no guard)
        const bool ended = p.endIfOurs();
        check(ended, "premise A3 needed: unguarded re-begin on the same target is indistinguishable (proposed ENDS it) -> documented as limit");
    }
    // ---- listener lifetime with the real hyprutils signal --------------------------------------------------------
    {
        CDragModel dc; g_dc = &dc; COwnership p; auto A = mk("A");
        p.begin(A, M_MOVE);
        p.endIfOurs();                     // handler resets its own listener during the emission
        check(p.handlerRuns == 1 && !p.listener, "listener reset inside its own handler: ran once, no crash (ASan), disarmed");
        const int before = p.handlerRuns;
        auto B = mk("B");
        dc.dragBegin(B, M_MOVE); dc.dragEnd(); // later sessions emit `ended`
        check(p.handlerRuns == before, "after disarm, later `ended` emissions do not call the plugin");
    }
    {
        // a listener registered during an emission is not called by that emission (snapshot)
        CDragModel dc; g_dc = &dc; int late = 0; CHyprSignalListener l2;
        auto l1 = dc.m_events.ended.listen([&] { l2 = dc.m_events.ended.listen([&] { ++late; }); });
        auto A = mk("A"); dc.dragBegin(A, M_MOVE); dc.dragEnd();
        check(late == 0, "hyprutils: listener added during emission is not invoked by it");
    }
    {
        // a listener dropped by its owner BEFORE its turn in the same emission is skipped (strongRef()==1)
        CDragModel dc; g_dc = &dc; int second = 0; CHyprSignalListener l2;
        auto l1 = dc.m_events.ended.listen([&] { l2.reset(); });
        l2 = dc.m_events.ended.listen([&] { ++second; });
        auto A = mk("A"); dc.dragBegin(A, M_MOVE); dc.dragEnd();
        check(second == 0, "hyprutils: listener released earlier in the same emission is skipped");
    }
    {
        // Q3 limit: if the module drops its listener from INSIDE an `ended` emission (e.g. unload triggered by another
        // listener), the emission's snapshot still holds the CSignalListener: its std::function is destroyed AFTER our
        // reset, i.e. after an unmap if unload were synchronous there. Measured with a destructor counter.
        struct SProbe { int* d = nullptr; ~SProbe() { if (d) ++*d; } }; // no temporary: only the shared instance counts
        CDragModel dc; g_dc = &dc; int destroyed = 0, destroyedAtReset = -1, oursRan = 0;
        CHyprSignalListener ours;
        // `other` is registered first, so it runs first in the emission and drops our listener ("unload" inside it)
        auto other = dc.m_events.ended.listen([&] { ours.reset(); destroyedAtReset = destroyed; });
        {
            auto probe = std::make_shared<SProbe>();
            probe->d   = &destroyed;
            ours       = dc.m_events.ended.listen([probe, &oursRan] { (void)probe; ++oursRan; });
        }
        auto A = mk("A"); dc.dragBegin(A, M_MOVE); dc.dragEnd();
        check(oursRan == 0 && destroyedAtReset == 0 && destroyed == 1,
              "Q3 limit reproduced: a plugin listener dropped inside an `ended` emission is skipped, but its handler state is destroyed only at the end of the emission (after the drop)");
    }
    // ---- unload step 2 ---------------------------------------------------------------------------------------------
    {
        CDragModel dc; g_dc = &dc; COwnership p; auto A = mk("A");
        p.begin(A, M_MOVE);
        p.unloadStep2();
        check(!dc.target() && !p.listener && dc.endedEmissions == 1, "unload step 2 with own drag: ended once, listener destroyed");
    }
    {
        CDragModel dc; g_dc = &dc; COwnership p; auto A = mk("A"), X = mk("X");
        p.begin(A, M_MOVE); dc.dragEnd(); dc.dragBegin(X, M_MOVE);
        p.unloadStep2();
        check(dc.target() == X && !p.listener, "unload step 2 with a third-party drag (after ours ended): untouched, listener destroyed");
        check(p.begin(A, M_MOVE) == REFUSED, "no start after unload step 2 began");
    }
    {
        // our target dies silently while we own it; unload must still destroy the armed listener
        CDragModel dc; g_dc = &dc; COwnership p; SPT A = mk("A");
        p.begin(A, M_MOVE);
        A = nullptr;
        p.unloadStep2();
        check(!p.listener && dc.endedEmissions == 0, "unload step 2 after a silent target death: no end attempted, listener destroyed anyway");
    }
    // Stale emission snapshot: the first listener starts a NEW plugin session while the old `ended` is being emitted.
    // Replacing the per-session listener drops its old owner ref; real hyprutils skips the old snapshot callback.
    {
        CDragModel dc; g_dc = &dc; COwnership p; auto A=mk("A"), B=mk("B");
        eBeginResult inner=REFUSED;
        auto first=dc.m_events.ended.listen([&] { inner=p.begin(B,M_MOVE); });
        p.begin(A,M_MOVE);
        dc.dragEnd();
        check(inner==STARTED && p.owns() && dc.target()==B && p.handlerRuns==0,
              "stale ended snapshot: old listener skipped after replacement; new session owns B, no old callback clears it");
        first.reset();
        p.endIfOurs();
        check(!p.owns() && !dc.target(), "new session from ended listener is ended normally on next release");
    }
    {
        CDragModel dc; g_dc=&dc; COwnership p; auto A=mk("A");
        p.begin(A,M_MOVE);
        const bool ended=p.endIfOurs();
        const auto r=ended?p.begin(A,M_RESIZE):REFUSED;
        check(ended && r==STARTED && p.owns() && dc.m_mode==M_RESIZE, "rearm: own end then begin acquires a fresh session");
        p.endIfOurs();
    }
    {
        CDragModel dc; g_dc=&dc; COwnership p; auto A=mk("A"), X=mk("X");
        p.begin(A,M_MOVE);
        dc.onFocusDuringEnd=[&] { dc.onFocusDuringEnd=nullptr; dc.dragBegin(X,M_MOVE); };
        const bool ended=p.endIfOurs();
        const auto r=ended?p.begin(A,M_RESIZE):REFUSED;
        check(ended && r==ALREADY && !p.owns() && dc.target()==X,
              "rearm: third-party start during end is kept; following begin does not adopt it");
    }
    // ---- initialization failure ------------------------------------------------------------------------------------
    {
        CDragModel dc; g_dc = &dc; COwnership p; auto A = mk("A");
        p.failListen = true;
        const auto R = p.begin(A, M_MOVE);
        check(R == STARTED && !p.owns() && !p.endIfOurs() && dc.target() == A, "listen() fails: drag started WITHOUT ownership (start behaviour kept); plugin never ends it");
    }
    fails += ReviewCases::reviewCases();
    std::printf("%d ok, %d failed\n", oks, fails);
    return fails ? 1 : 0;
}
