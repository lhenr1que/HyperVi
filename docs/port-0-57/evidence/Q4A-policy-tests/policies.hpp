// Q4-A end-policy candidates (host-free; NOT production code). Every policy sees the host only through the observable
// section of CHost (target(), mode(), wasDraggingWindow(), m_events.ended, beginDragTarget, endDragTarget,
// decorationsTakeDragStart, idleQueue). PolHook additionally reads hook counters that do NOT exist in the 0.57 API: it
// stands for an architectural alternative (function hooks), not for something the plugin can do today.
#pragma once
#include "host.hpp"

enum eBeginResult { STARTED, ALREADY, REFUSED, REFUSED_BY_DECO };
inline const char* beginName(eBeginResult r) {
    static const char* N[] = {"STARTED", "ALREADY", "REFUSED", "REFUSED_BY_DECO"};
    return N[r];
}

// Production start helper (plugin/hostDrag.hpp, P8-B): refuse if any drag, ask decorations for DRAG_START, then begin.
inline eBeginResult kwDragBegin(CHost& h, SPT t, eMode m) {
    if (h.target())
        return ALREADY;
    if (m == M_MOVE && h.decorationsTakeDragStart(t))
        return REFUSED_BY_DECO;
    h.beginDragTarget(t, m);
    return STARTED;
}

// Variant of the helper with target() re-checked right before beginDragTarget (would need a P8-B helper change; not
// authorized here). Closes the window between the entry guard and dragBegin in which DRAG_START decorations run.
inline eBeginResult kwDragBeginRechecked(CHost& h, SPT t, eMode m) {
    if (h.target())
        return ALREADY;
    if (m == M_MOVE && h.decorationsTakeDragStart(t))
        return REFUSED_BY_DECO;
    if (h.target())
        return ALREADY;
    h.beginDragTarget(t, m);
    return STARTED;
}

struct IPolicy {
    explicit IPolicy(CHost& h_) : h(h_) {}
    virtual ~IPolicy()                              = default;
    virtual const char*  name() const               = 0;
    virtual eBeginResult begin(SPT t, eMode m)      = 0; // K1/K2
    virtual void         requestEnd()               = 0; // K3/K9
    virtual void         requestRearm(SPT t, eMode m) { requestEnd(); (void)begin(t, m); }
    virtual void         unload()                   = 0; // PLUGIN_EXIT step 2
    virtual bool         owns() const               = 0;
    CHost&               h;
};

// Counterfactual and liveness control: starts like production, never ends anything.
struct PolNone : IPolicy {
    using IPolicy::IPolicy;
    const char*  name() const override { return "none"; }
    eBeginResult begin(SPT t, eMode m) override { return kwDragBegin(h, t, m); }
    void         requestEnd() override {}
    void         requestRearm(SPT, eMode) override {}
    void         unload() override {}
    bool         owns() const override { return false; }
};

// Legacy idea: end when the current target is the one we started.
struct PolTarget : IPolicy {
    using IPolicy::IPolicy;
    WPT          remembered;
    const char*  name() const override { return "target-eq"; }
    eBeginResult begin(SPT t, eMode m) override {
        const auto r = kwDragBegin(h, t, m);
        remembered   = t;
        return r;
    }
    bool owns() const override { return h.target() && h.target() == remembered.lock(); }
    void requestEnd() override {
        if (owns())
            h.endDragTarget();
    }
    void unload() override { requestEnd(); }
};

// Previous Q4-A follow-up contract C1-C4 (session listener + generation + acquisition token + inBegin/inOwnEnd),
// acquisition by post-check target()==t. Kept to show which new scenarios it fails.
struct PolC3 : IPolicy {
    using IPolicy::IPolicy;
    uint64_t                generation = 0, acquisition = 0;
    std::optional<uint64_t> ownedAt;
    WPT                     ownedTarget;
    CHyprSignalListener     listener;
    bool                    inBegin = false, inOwnEnd = false, unloading = false;
    const char*             name() const override { return "C3-prev"; }
    void                    disarm() { ++acquisition; ownedAt.reset(); ownedTarget.reset(); listener.reset(); }
    bool owns() const override { return listener && ownedAt && *ownedAt == generation && !ownedTarget.expired() && h.target() == ownedTarget.lock(); }
    eBeginResult begin(SPT t, eMode m) override {
        if (unloading || inOwnEnd || inBegin) return REFUSED;
        if (h.target()) return ALREADY;
        inBegin = true;
        Hyprutils::Utils::CScopeGuard g([this] { inBegin = false; });
        const uint64_t tok = ++acquisition;
        listener           = h.m_events.ended.listen([this, tok] { if (tok != acquisition) return; ++generation; disarm(); });
        const uint64_t G0  = generation;
        const auto     r   = kwDragBegin(h, t, m);
        if (generation != G0 || !h.target() || h.target() != t) { disarm(); return r == STARTED ? REFUSED : r; }
        ownedAt = G0; ownedTarget = t;
        return STARTED; // NOTE: ignores r (a decoration may have started the drag on t)
    }
    void requestEnd() override {
        if (!owns()) return;
        inOwnEnd = true;
        h.endDragTarget();
        inOwnEnd = false;
        if (ownedAt) disarm();
    }
    void unload() override { unloading = true; if (owns()) requestEnd(); disarm(); }
};

// ===== CANDIDATE: verifiable ownership + ambiguity veto + consume-before-end =========================================
// Ownership proof obligations (all must hold at the instant of the end call):
//   (a) acquired only when the plugin's OWN beginDragTarget ran (helper result STARTED), no `ended` was observed
//       during the begin and the controller's target is the requested one afterwards;
//   (b) the session listener armed for that acquisition is still installed and its token is current (no `ended`
//       delivered to it since); callbacks with an old token are ignored;
//   (c) the acquired target is alive and is the controller's target;
//   (d) the host is NOT inside the mapped-window callback span of a dragEnd: wasDraggingWindow() is false
//       (U:275 sets it after the target reset, :288/:325 clear it before the emission);
//   (e) no other plugin end/begin transaction is running (non-reentrant), and ownership is CONSUMED before calling
//       endDragTarget, so nothing reached synchronously from inside that call can reuse it.
struct PolVeto : IPolicy {
    using IPolicy::IPolicy;
    uint64_t                generation = 0, acquisition = 0;
    std::optional<uint64_t> ownedAt;
    WPT                     ownedTarget;
    CHyprSignalListener     listener;
    bool                    inBegin = false, inEnd = false, unloading = false;
    bool                    recheck = false; // helper variant (see kwDragBeginRechecked)
    int                     vetoes = 0;
    const char*             name() const override { return "veto"; }
    void                    disarm() { ++acquisition; ownedAt.reset(); ownedTarget.reset(); listener.reset(); } // veto: token + listener
    bool                    ambiguous() const { return h.wasDraggingWindow(); }
    bool                    owns() const override {
        if (!listener || !ownedAt || *ownedAt != generation) return false;     // (b)
        if (ownedTarget.expired() || h.target() != ownedTarget.lock()) return false; // (c)
        return true;
    }
    eBeginResult begin(SPT t, eMode m) override {
        if (unloading || inBegin || inEnd) return REFUSED;                 // (e)
        if (ambiguous()) { ++vetoes; return REFUSED; }                     // (d) do not start inside a host end
        if (h.target()) return ALREADY;
        inBegin = true;
        Hyprutils::Utils::CScopeGuard g([this] { inBegin = false; });
        const uint64_t tok = ++acquisition;
        listener           = h.m_events.ended.listen([this, tok] {
            if (tok != acquisition) return;                                // (b) stale snapshot callback
            ++generation;
            disarm();
        });
        const uint64_t G0 = generation;
        const auto     r  = recheck ? kwDragBeginRechecked(h, t, m) : kwDragBegin(h, t, m);
        if (r != STARTED) { disarm(); return r; }                          // (a) e.g. the group bar took the drag
        if (generation != G0 || h.target() != t) { disarm(); return REFUSED; } // (a)
        ownedAt     = G0;
        ownedTarget = t;
        return STARTED;
    }
    bool endOwned() {
        if (inEnd || inBegin) return false;                                // (e)
        if (ambiguous()) { ++vetoes; return false; }                       // (d)
        if (!owns()) return false;
        disarm();                                                          // (e) consume before the call
        inEnd = true;
        Hyprutils::Utils::CScopeGuard g([this] { inEnd = false; });
        return h.endDragTarget();
    }
    void requestEnd() override { (void)endOwned(); }
    void requestRearm(SPT t, eMode m) override {
        if (endOwned()) (void)begin(t, m);
    }
    void unload() override { unloading = true; (void)endOwned(); disarm(); }
};

// Candidate + helper that re-checks target() after the DRAG_START dispatch (review 1 major).
struct PolVetoRecheck : PolVeto {
    explicit PolVetoRecheck(CHost& h_) : PolVeto(h_) { recheck = true; }
    const char* name() const override { return "veto-rc"; }
};

// ===== ALTERNATIVE (needs deferred work = lifecycle authorization): same proof, end executed at idle ================
// End entries only RECORD the request with the current acquisition token; an idle callback (doLater) re-checks
// ownership when no dragEnd frame and no `ended` emission is on the stack, so every `ended` of a finished end has been
// delivered. Must be cancelled in PLUGIN_EXIT (removeDoLater/doLaterLock) - not authorized in this step.
struct PolDefer : PolVeto {
    using PolVeto::PolVeto;
    std::optional<uint64_t> pendingFor;
    std::optional<eMode>    pendingRearm;
    SPT                     rearmTarget;
    bool                    scheduled = false;
    const char*             name() const override { return "defer"; }
    void                    schedule() {
        if (scheduled) return;
        scheduled = true;
        h.idleQueue.push_back([this] {
            CHost::CallerScope s(h, W_KWINISH);
            scheduled      = false;
            const auto tok = pendingFor;
            const auto rm  = pendingRearm;
            auto       rt  = rearmTarget;
            pendingFor.reset(); pendingRearm.reset(); rearmTarget = nullptr;
            if (!tok || *tok != acquisition) return; // ownership changed since the request
            if (endOwned() && rm) (void)begin(rt, *rm);
        });
    }
    void requestEnd() override {
        if (unloading || !owns()) return;
        pendingFor = acquisition;
        schedule();
    }
    void requestRearm(SPT t, eMode m) override {
        if (unloading || !owns()) return;
        pendingFor = acquisition; pendingRearm = m; rearmTarget = t;
        schedule();
    }
    void unload() override { pendingFor.reset(); pendingRearm.reset(); rearmTarget = nullptr; PolVeto::unload(); }
};

// ===== ALTERNATIVE (needs function hooks on dragBegin/dragEnd = hook + lifecycle authorization) =====================
// A hook gives a real session identity: any begin or end after the acquisition changes the sequence.
struct PolHook : IPolicy {
    using IPolicy::IPolicy;
    std::optional<uint64_t> ownedSeq;
    WPT                     ownedTarget;
    bool                    inBegin = false, inEnd = false, unloading = false;
    const char*             name() const override { return "hook"; }
    bool owns() const override { return ownedSeq && *ownedSeq == h.hookSeq() && !ownedTarget.expired() && h.target() == ownedTarget.lock(); }
    eBeginResult begin(SPT t, eMode m) override {
        if (unloading || inBegin || inEnd) return REFUSED;
        if (h.target()) return ALREADY;
        inBegin = true;
        Hyprutils::Utils::CScopeGuard g([this] { inBegin = false; });
        const uint64_t S0 = h.hookSeq();
        const auto     r  = kwDragBegin(h, t, m);
        if (r != STARTED) return r;
        if (h.hookSeq() != S0 + 1 || h.target() != t) return REFUSED; // anything else began/ended meanwhile
        ownedSeq    = h.hookSeq();
        ownedTarget = t;
        return STARTED;
    }
    bool endOwned() {
        if (inEnd || inBegin || !owns()) return false;
        ownedSeq.reset();
        inEnd = true;
        Hyprutils::Utils::CScopeGuard g([this] { inEnd = false; });
        return h.endDragTarget();
    }
    void requestEnd() override { (void)endOwned(); }
    void requestRearm(SPT t, eMode m) override { if (endOwned()) (void)begin(t, m); }
    void unload() override { unloading = true; (void)endOwned(); ownedSeq.reset(); }
};
