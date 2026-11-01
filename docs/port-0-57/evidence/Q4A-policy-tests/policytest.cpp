// Q4-A end-policy study: scenarios, two policy-independent oracles (O1 event-level improper end/replacement, O2 liveness
// of provably safe own ends) and one DIFFERENTIAL check (O3: final state vs. the never-ending run; not causal).
// Host-free; NOT production code.
// Build: g++/clang++ -std=c++2b -fsanitize=address,undefined policytest.cpp $(pkg-config --cflags --libs hyprutils)
#include "policies.hpp"

#include <cstring>
#include <map>
#include <memory>
#include <set>

// ---------------------------------------------------------------------------------------------------------------------
// Driver: plugin entries are attributed to Kwinish; foreign code pushes its own identity.
struct Ctx {
    CHost&                   h;
    IPolicy&                 p;
    std::vector<std::string> spec; // scenario-specific contract checks (column s)
    void expect(bool ok, const char* what) { if (!ok) spec.push_back(what); }
    SPT      mk(const char* n) {
        auto t  = makeShared<STarget>();
        t->name = n;
        h.windows.push_back(t);
        return t;
    }
    eBeginResult kwBegin(SPT t, eMode m = M_MOVE) {
        CHost::CallerScope s(h, W_KWINISH);
        return p.begin(t, m);
    }
    void kwEnd(const char* where) { // K3/K9 entry (button release, unsnap pause, menu-grab click...)
        Truth::noteEndRequest(h, where);
        CHost::CallerScope s(h, W_KWINISH);
        p.requestEnd();
    }
    void kwRearm(SPT t, const char* where) { // rearm entry (Shift / unsnap restart)
        Truth::noteEndRequest(h, where);
        CHost::CallerScope s(h, W_KWINISH);
        p.requestRearm(t, M_MOVE);
    }
    void kwUnload() {
        Truth::noteEndRequest(h, "unload step 2");
        CHost::CallerScope s(h, W_KWINISH);
        p.unload();
    }
    void hostEnd() { // key / button / floatWindow / Actions::mouse / unmap (F7)
        CHost::CallerScope s(h, W_HOST);
        h.endDragTarget();
    }
    void guardedBegin(eWho w, SPT t, eMode m = M_MOVE) { // every 0.57 host caller checks target() first (F2/A3)
        CHost::CallerScope s(h, w);
        if (!h.target()) h.beginDragTarget(t, m);
    }
    void foreignEnd(eWho w) {
        CHost::CallerScope s(h, w);
        h.endDragTarget();
    }
    void groupBarStart(eWho w, SPT t) { // Actions::mouse over a group bar (ConfigActions.cpp:1704,1733)
        CHost::CallerScope s(h, w);
        if (!h.target()) (void)h.decorationsTakeDragStart(t);
    }
};

struct SScenario {
    const char*                     name;
    const char*                     what;
    std::function<void(Ctx&)>       body;
    std::vector<CHyprSignalListener> keep; // unused placeholder
};

// ---------------------------------------------------------------------------------------------------------------------
struct SFinal {
    bool        has = false;
    eWho        who = W_HOST;
    std::string target;
    int         ordinal = 0;
    eMode       mode    = M_INVALID;
    bool        wasDragging = false;
    std::string str() const {
        if (!has) return std::string("no drag") + (wasDragging ? " wasDragging=1" : "");
        return std::string(whoName(who)) + ":" + target + "#" + std::to_string(ordinal) + " mode=" + std::to_string(mode) + (wasDragging ? " wasDragging=1" : "");
    }
};
struct SResult {
    std::vector<std::string> o1, o2, o3, spec;
    std::string              orphan;
    SFinal                   fin;
    std::string              note;
};

template <typename P>
static SResult runOne(const SScenario& sc) {
    CHost h;
    P     p(h);
    Ctx   c{h, p, {}};
    sc.body(c);
    h.runIdle(); // the event loop returns to idle at the end of the scenario
    SResult r;
    r.spec = c.spec;
    // O1: no improper end/replacement requested by Kwinish (truth: caller + session initiator at the dragEnd entry)
    r.o1 = Truth::violations(h);
    // O2: every provably safe own end request was honoured (possibly at idle)
    for (const auto& q : Truth::requests(h))
        if (q.livenessRequired && q.session && !Truth::sessions(h)[*q.session].endedByKwinish)
            r.o2.push_back(std::string("own clean session not ended after request at '") + q.where + "'");
    if (auto s = Truth::current(h)) {
        r.fin = {true, s->who, s->target, s->ordinal, h.mode(), h.wasDraggingWindow()};
        if (s->who == W_KWINISH && !p.owns()) r.orphan = "kwinish session " + s->target + " alive without ownership";
    } else
        r.fin.wasDragging = h.wasDraggingWindow();
    return r;
}

// O3: compare the final host state with the counterfactual run in which Kwinish never ends anything (PolNone).
static void oracle3(SResult& r, const SResult& none) {
    const auto& P = r.fin;
    const auto& N = none.fin;
    if (P.wasDragging != N.wasDragging) r.o3.push_back("wasDraggingWindow differs from counterfactual");
    if (N.has && N.who != W_KWINISH) {
        if (!P.has || P.who != N.who || P.target != N.target || P.ordinal != N.ordinal || P.mode != N.mode)
            r.o3.push_back("foreign session " + N.str() + " (counterfactual) became " + P.str());
    } else if (!N.has && P.has && P.who != W_KWINISH)
        r.o3.push_back("foreign session " + P.str() + " exists only with the policy");
    // attribution of the final host state
    if (P.has && P.mode == M_INVALID) {
        const bool same = N.has && N.who == P.who && N.target == P.target && N.ordinal == P.ordinal && N.mode == P.mode;
        if (same)
            r.note = std::string("compositor-caused: ") + whoName(P.who) + " session left with mode INVALID by the host; identical in the run without Kwinish ends";
        else if (r.o1.empty())
            r.note = std::string("host-caused inside an end Kwinish requested for ITS OWN session: ") + whoName(P.who) +
                " session left with mode INVALID by the enclosing dragEnd (R3 host limit; O1 shows no foreign session was ended)";
        else
            r.note = "differs from the run without Kwinish ends; caused by the improper end reported by O1";
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static std::vector<SScenario> scenarios() {
    std::vector<SScenario> v;
    auto add = [&](const char* n, const char* w, std::function<void(Ctx&)> b) { v.push_back({n, w, std::move(b), {}}); };

    // ---- own drag, normal paths ----
    add("N1.own.release", "own drag, release at top level", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        c.kwEnd("release");
    });
    add("N2.own.rearm", "own drag, rearm (end+begin), then release", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        c.kwRearm(A, "rearm");
        c.h.runIdle();
        c.kwEnd("release after rearm");
    });
    add("N3.own.restart", "host ends own drag (key), later Kwinish starts again and releases", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        c.hostEnd();
        c.kwBegin(A);
        c.kwEnd("release of restarted drag");
    });
    // ---- external drags ----
    add("E1.ext.same-window", "host ends own; user drag on SAME window; stale Kwinish release (S2)", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        c.hostEnd();
        c.guardedBegin(W_USER, A);
        c.kwEnd("stale release");
    });
    add("E2.ext.already", "third-party drag active; Kwinish begin refused; release", [](Ctx& c) {
        auto A = c.mk("A"), X = c.mk("X");
        c.guardedBegin(W_THIRD, X);
        c.kwBegin(A);
        c.kwEnd("release");
    });
    // ---- R1-R3 (review regressions) ----
    for (bool same : {false, true})
        add(same ? "R1.same" : "R1.other", "begin refused inside host -> ended -> earlier listener calls Kwinish begin", [same](Ctx& c) {
            auto A = c.mk("A"), B = same ? A : c.mk("B");
            A->mapped = false;
            auto once = std::make_shared<bool>(false);
            auto L    = std::make_shared<CHyprSignalListener>();
            *L        = c.h.m_events.ended.listen([&c, B, once] {
                if (*once) return;
                *once     = true;
                B->mapped = true;
                c.kwBegin(B);
            });
            c.kwBegin(A);
            L->reset();
            c.kwEnd("release");
        });
    add("R2.nested-snapshots", "listener registered BEFORE Kwinish: ended -> X begin/end -> nested ended -> Kwinish acquires B", [](Ctx& c) {
        auto A = c.mk("A"), X = c.mk("X"), B = c.mk("B");
        auto level = std::make_shared<int>(0);
        auto L     = std::make_shared<CHyprSignalListener>();
        *L = c.h.m_events.ended.listen([&c, X, B, level] {
            if (*level == 0) { *level = 1; c.guardedBegin(W_THIRD, X); c.foreignEnd(W_THIRD); }
            else if (*level == 1) { *level = 2; c.kwBegin(B); }
        });
        c.kwBegin(A); // its per-session listener is AFTER L in both snapshots
        c.hostEnd();
        L->reset();
        c.kwEnd("release B");
    });
    add("R3.mode-clobber", "third-party starts X inside host end of own drag", [](Ctx& c) {
        auto A = c.mk("A"), X = c.mk("X");
        c.kwBegin(A);
        c.h.onFocus = [&c, X] { c.h.onFocus = nullptr; c.guardedBegin(W_THIRD, X); };
        c.hostEnd();
        c.kwEnd("release");
    });
    // ---- R4: target reset and `ended` are not atomic; routes inside the mapped callback span (wasDragging=1) ----
    struct SRoute { const char* n; int k; };
    for (auto [rn, k] : {SRoute{"R4.host.focus", 0}, SRoute{"R4.host.deco-continue", 1}, SRoute{"R4.host.deco-return", 2}, SRoute{"R4.host.group-add", 3},
                         SRoute{"R4.host.floating", 4}, SRoute{"R4.host.geometry", 5}, SRoute{"R4.own.focus", 6}, SRoute{"R4.own.deco-return", 7}, SRoute{"R4.host.rearm", 8}})
        add(rn, "re-begin on SAME target + Kwinish end entry inside the host end", [k](Ctx& c) {
            auto A   = c.mk("A");
            auto hit = [&c, A, k] { c.guardedBegin(W_THIRD, A); if (k == 8) c.kwRearm(A, "rearm inside end window"); else c.kwEnd("inside end window"); };
            if (k == 3) A->joinsGroup = true;
            if (k == 4) A->draggingTiled = true;
            c.kwBegin(A);
            auto once = std::make_shared<bool>(false);
            auto h1   = [hit, once] { if (!*once) { *once = true; hit(); } };
            switch (k) {
                case 0: case 6: case 8: c.h.onFocus = h1; break;
                case 1: c.h.onDecoDragEnd = [h1] { h1(); return false; }; break;
                case 2: case 7: c.h.onDecoDragEnd = [h1] { h1(); return true; }; break;
                case 3: c.h.onGroupAdd = h1; break;
                case 4: c.h.onFloatingChange = h1; break;
                case 5: c.h.onSetTargetGeom = h1; break;
            }
            if (k == 6 || k == 7) c.kwEnd("release (own end)");
            else c.hostEnd();
        });
    add("R4.nested-clear", "inside host end: X begin/end (nested ended clears wasDragging), re-begin A, Kwinish end", [](Ctx& c) {
        auto A = c.mk("A"), X = c.mk("X");
        c.kwBegin(A);
        c.h.onFocus = [&c, A, X] {
            c.h.onFocus = nullptr;
            c.guardedBegin(W_THIRD, X);
            c.foreignEnd(W_THIRD);
            c.guardedBegin(W_THIRD, A);
            c.kwEnd("after nested end, still inside outer end");
        };
        c.hostEnd();
    });
    // ---- during the `ended` emission (after :325, wasDragging=0): foreign listener registered BEFORE Kwinish ----
    for (int k : {0, 1, 2})
        add(k == 0 ? "R4.emit.host" : k == 1 ? "R4.emit.own" : "M2.emit.no-rebegin",
            k == 2 ? "foreign ended listener triggers a Kwinish end entry without re-begin"
                   : "foreign ended listener (registered first) re-begins SAME target and triggers a Kwinish end entry",
            [k](Ctx& c) {
                auto A    = c.mk("A");
                auto once = std::make_shared<bool>(false);
                auto L    = std::make_shared<CHyprSignalListener>();
                *L        = c.h.m_events.ended.listen([&c, A, once, k] {
                    if (*once) return;
                    *once = true;
                    if (k != 2) c.guardedBegin(W_THIRD, A);
                    c.kwEnd("inside ended emission");
                });
                c.kwBegin(A);
                if (k == 1) c.kwEnd("release (own end)");
                else c.hostEnd();
                L->reset();
            });
    add("M3.emit.acquire-then-release", "Kwinish begins B inside an ended emission (stale snapshot), releases at top level", [](Ctx& c) {
        auto A = c.mk("A"), B = c.mk("B");
        auto once = std::make_shared<bool>(false);
        auto L    = std::make_shared<CHyprSignalListener>();
        *L        = c.h.m_events.ended.listen([&c, B, once] { if (!*once) { *once = true; c.kwBegin(B); } });
        c.kwBegin(A);
        c.hostEnd();
        L->reset();
        c.kwEnd("release B");
    });
    // ---- special exits ----
    add("X1.silent-death", "own target destroyed (no ended), host end no-op, third drag X, release", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        c.h.windows.clear();
        A = nullptr;
        c.hostEnd();
        c.guardedBegin(W_THIRD, c.mk("X"));
        c.kwEnd("release");
    });
    add("X2.ended-without-reset", "unmapped target without window: ended emitted, target kept (F5)", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        A->mapped = false; A->hasWindow = false;
        c.hostEnd();
        c.kwEnd("release");
    });
    for (int v = 0; v < 4; ++v)
        add(v == 0 ? "X3.refused.unmapped" : v == 1 ? "X3.refused.no-workspace" : v == 2 ? "X3.refused.fs-reject" : "X3.refused.no-window",
            "begin refused inside dragBegin (ended inside begin); later third drag; release", [v](Ctx& c) {
                auto A = c.mk("A");
                if (v == 0) A->mapped = false;
                if (v == 1) A->hasWorkspace = false;
                if (v == 2) A->fsWorkspaceReject = true;
                if (v == 3) { A->mapped = false; A->hasWindow = false; }
                const auto r = c.kwBegin(A);
                c.expect(r != STARTED || Truth::startedCleanly(c.h, W_KWINISH), "STARTED only for a Kwinish session that began without an end/refusal");
                if (v != 3) c.guardedBegin(W_THIRD, c.mk("X"));
                c.kwEnd("release");
            });
    add("X4.fs-reject-callback", "refusal dragEnd inside Kwinish begin runs callbacks: third re-begins A + Kwinish end entry", [](Ctx& c) {
        auto A = c.mk("A");
        A->fsWorkspaceReject = true;
        c.h.onFocus = [&c, A] { c.h.onFocus = nullptr; A->fsWorkspaceReject = false; c.guardedBegin(W_THIRD, A); c.kwEnd("inside refusal end"); };
        const auto r = c.kwBegin(A);
        c.expect(r != STARTED || Truth::startedCleanly(c.h, W_KWINISH), "STARTED only for a Kwinish session that began without an end/refusal");
        c.kwEnd("release");
    });
    // ---- group bar ----
    add("G1.kwbegin-groupbar-same", "Kwinish begin over group bar: bar starts its own drag on the SAME window", [](Ctx& c) {
        auto A = c.mk("A");
        A->groupBarStart = true;
        c.kwBegin(A);
        c.kwEnd("release");
    });
    add("H1.helper.deco-start-false", "Kwinish begin: a third-party DRAG_START decoration starts its own drag and returns false (review 1)", [](Ctx& c) {
        auto A = c.mk("A");
        A->foreignDecoStartReturnsFalse = true;
        c.kwBegin(A);
        c.kwEnd("release");
    });
    add("G1.kwbegin-groupbar-other", "Kwinish begin over group bar: bar starts a drag on another member", [](Ctx& c) {
        auto A = c.mk("A");
        c.mk("B");
        A->groupBarStart = true; A->groupBarStartsOn = "B";
        c.kwBegin(A);
        c.kwEnd("release");
    });
    add("G2.drop-into-group", "own release drops into a group: group add + focus inside end; third re-begins A + Kwinish entry", [](Ctx& c) {
        auto A = c.mk("A");
        A->joinsGroup = true;
        c.kwBegin(A);
        c.h.onGroupAdd = [&c, A] { c.h.onGroupAdd = nullptr; c.guardedBegin(W_THIRD, A); c.kwEnd("inside group add"); };
        c.kwEnd("release (own end)");
    });
    add("G3.groupbar-by-user", "host ended own; user drags a tab from the group bar (same window); stale release", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        c.hostEnd();
        A->groupBarStart = true;
        c.groupBarStart(W_USER, A);
        c.kwEnd("stale release");
    });
    add("G4.groupbar-inside-end", "inside host end: Lua/user starts a group-bar drag on SAME window; Kwinish end entry", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        A->groupBarStart = true;
        c.h.onFocus = [&c, A] { c.h.onFocus = nullptr; c.groupBarStart(W_USER, A); c.kwEnd("inside end window"); };
        c.hostEnd();
    });
    add("G5.groupbar-focus-kwbegin", "group bar start (by user) focuses window; Kwinish begin entry from that focus", [](Ctx& c) {
        auto A = c.mk("A");
        A->groupBarStart = true;
        c.h.onGroupBarFocus = [&c, A] { c.h.onGroupBarFocus = nullptr; c.kwBegin(A); };
        c.groupBarStart(W_USER, A);
        c.kwEnd("release");
    });
    // ---- begin inside a host end ----
    add("B1.kwbegin-inside-end", "Kwinish begin entry runs inside the host end of a third drag", [](Ctx& c) {
        auto A = c.mk("A"), X = c.mk("X");
        c.guardedBegin(W_THIRD, X);
        c.h.onFocus = [&c, A] { c.h.onFocus = nullptr; c.kwBegin(A); };
        c.hostEnd();
        c.kwEnd("release");
    });
    // ---- unload ----
    add("U1.unload.own", "unload step 2 with own drag", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        c.kwUnload();
    });
    add("U2.unload.third", "unload with a user drag on the same window after own ended", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        c.hostEnd();
        c.guardedBegin(W_USER, A);
        c.kwUnload();
    });
    add("U3.unload.then-begin", "begin after unload step 2 started", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwUnload();
        const auto r = c.kwBegin(A);
        c.expect(r != STARTED && !c.h.target(), "no Kwinish drag starts after unload step 2");
    });
    // ---- premise A3 (outside the guarded domain) ----
    add("L1.A3.unguarded", "third party calls dragBegin on SAME target while own drag is active (violates A3)", [](Ctx& c) {
        auto A = c.mk("A");
        c.kwBegin(A);
        { CHost::CallerScope s(c.h, W_THIRD); c.h.beginDragTarget(A, M_RESIZE); }
        c.kwEnd("release");
    });
    return v;
}

// ---------------------------------------------------------------------------------------------------------------------
template <typename P>
static std::map<std::string, SResult> runAll(const std::vector<SScenario>& sc, const std::map<std::string, SResult>* none) {
    std::map<std::string, SResult> out;
    for (auto& s : sc) {
        auto r = runOne<P>(s);
        if (none) oracle3(r, none->at(s.name));
        out[s.name] = r;
    }
    return out;
}

int main(int argc, char** argv) {
    const bool verbose = argc > 1 && !std::strcmp(argv[1], "-v");
    const auto sc      = scenarios();
    auto       none    = runAll<PolNone>(sc, nullptr);
    for (auto& [k, r] : none) oracle3(r, none.at(k));
    std::vector<std::pair<std::string, std::map<std::string, SResult>>> all;
    all.push_back({"none", none});
    all.push_back({"target-eq", runAll<PolTarget>(sc, &none)});
    all.push_back({"C3-prev", runAll<PolC3>(sc, &none)});
    all.push_back({"veto", runAll<PolVeto>(sc, &none)});
    all.push_back({"veto-rc", runAll<PolVetoRecheck>(sc, &none)});
    all.push_back({"defer", runAll<PolDefer>(sc, &none)});
    all.push_back({"hook", runAll<PolHook>(sc, &none)});

    std::printf("%-30s", "scenario \\ policy");
    for (auto& [n, _] : all) std::printf(" %-10s", n.c_str());
    std::printf("\n");
    for (auto& s : sc) {
        std::printf("%-30s", s.name);
        for (auto& [n, m] : all) {
            const auto& r = m.at(s.name);
            std::string cell = std::string(r.o1.empty() ? "." : "1") + (r.o2.empty() ? "." : "2") + (r.o3.empty() ? "." : "3") + (r.spec.empty() ? "" : "s") + (r.orphan.empty() ? "" : "o");
            std::printf(" %-10s", cell.c_str());
        }
        std::printf("\n");
    }
    std::printf("legend: 1 = O1 improper Kwinish end, 2 = O2 safe own end not honoured, 3 = O3 final state differs from counterfactual, s = scenario contract check failed, o = Kwinish session alive without ownership (info)\n");
    // machine-readable lines + details
    for (auto& [n, m] : all)
        for (auto& s : sc) {
            const auto& r = m.at(s.name);
            std::printf("RES %s %s O1=%d O2=%d O3=%d spec=%d orphan=%d final=[%s]%s%s\n", n.c_str(), s.name, !r.o1.empty(), !r.o2.empty(), !r.o3.empty(), !r.spec.empty(), !r.orphan.empty(), r.fin.str().c_str(),
                        r.note.empty() ? "" : " NOTE: ", r.note.c_str());
            if (verbose || n == "veto" || n == "veto-rc" || n == "defer")
                for (auto* vec : {&r.o1, &r.o2, &r.o3, &r.spec})
                    for (auto& e : *vec) std::printf("    %s/%s: %s\n", n.c_str(), s.name, e.c_str());
        }
    return 0;
}
