// Q4-A end-policy study: host-free model of the Hyprland 0.57 drag controller (NOT production code).
// Source pinned: U = Hyprland 0aa6745f, src/layout/supplementary/DragController.cpp
//   sha256 6a7c339a4519dcd7787596cd00a3b7931451e770aff1c347242d87427dc30a5f (same file the earlier Q4-A model cites).
// Signals are the REAL Hyprutils::Signal::CSignalT<> / CHyprSignalListener (installed hyprutils 0.14.3).
//
// Two layers, kept apart on purpose:
//   * OBSERVABLE: what a 0.57 plugin can read or call (public members of CDragStateController / CLayoutManager and the
//     event-bus callbacks the scenario wires in). Policies only receive `CHost&` and may use ONLY the members in the
//     "observable" section; the "truth" section is private and read by the oracles through `Truth`.
//   * TRUTH: information that exists only inside the compositor (or only in this test): session identity, who called
//     dragBegin/dragEnd, whether a dragEnd frame or an `ended` emission is on the stack. Oracles use it; policies cannot.
#pragma once
#include <hyprutils/memory/SharedPtr.hpp>
#include <hyprutils/memory/WeakPtr.hpp>
#include <hyprutils/signal/Signal.hpp>
#include <hyprutils/utils/ScopeGuard.hpp>

#include <cstdint>
#include <cstdio>
#include <functional>
#include <optional>
#include <string>
#include <vector>

using Hyprutils::Memory::CSharedPointer;
using Hyprutils::Memory::CWeakPointer;
using Hyprutils::Memory::makeShared;
using Hyprutils::Signal::CHyprSignalListener;
using Hyprutils::Signal::CSignalT;

enum eMode : int8_t { M_INVALID = -1, M_MOVE = 0, M_RESIZE = 1 };
// who is executing a call into the controller (truth only)
enum eWho : uint8_t { W_HOST = 0, W_USER, W_THIRD, W_GROUPBAR, W_KWINISH };
inline const char* whoName(eWho w) {
    static const char* N[] = {"host", "user", "third", "groupbar", "kwinish"};
    return N[w];
}

struct STarget {
    std::string name;
    bool        mapped = true, hasWorkspace = true, hasWindow = true, fsWorkspaceReject = false;
    bool        overOtherWindow = true; // :283-286 a window is under the cursor at drop (decorations/group checks run)
    bool        joinsGroup      = false; // :298-302 drag_into_group adds the dragged window to the group under the cursor
    bool        draggingTiled   = false; // :311-317 drag started on a tiled window (changeFloatingMode at the end)
    // group bar decoration on THIS window: DRAG_START handler starts its own drag (CHyprGroupBarDecoration.cpp:392-423)
    bool        groupBarStart = false;
    // review 1 (major): a third-party DRAG_START decoration that starts its own drag but returns false
    bool        foreignDecoStartReturnsFalse = false;
    std::string groupBarStartsOn; // empty = this window (the dragged tab is the active one); otherwise another member
};
using SPT = CSharedPointer<STarget>;
using WPT = CWeakPointer<STarget>;

struct SSession { // truth
    uint64_t    id = 0;
    eWho        who = W_HOST;
    std::string target;
    int         ordinal = 0;          // n-th session of (who,target): stable key across runs
    uint64_t    cleanMark = 0;        // value of `emissions` when its begin returned
    bool        endedByKwinish = false;
    bool        endedByOther   = false;
};

struct SEndRequest { // truth: context of a Kwinish end entry (K3/K9/rearm)
    int                   endDepth = 0, emitDepth = 0;
    std::optional<size_t> session; // index into sessions, if a live drag existed
    bool                  livenessRequired = false;
    std::string           where;
};

class CHost {
  public:
    // =========================== OBSERVABLE (0.57 public API) ===========================================
    SPT   target() const { return m_target.lock(); }       // CDragStateController::target()
    eMode mode() const { return m_dragMode; }               // ::mode()
    bool  wasDraggingWindow() const { return m_wasDraggingWindow; } // ::wasDraggingWindow()
    struct {
        CSignalT<> ended;                                    // ::m_events.ended (no payload)
    } m_events;
    void beginDragTarget(SPT t, eMode m) { dragBegin(t, m); } // CLayoutManager::beginDragTarget -> dragBegin
    bool endDragTarget() { return dragEnd(); }                // CLayoutManager::endDragTarget -> dragEnd
    // checkInputOnDecorations(INPUT_TYPE_DRAG_START) on the window under the cursor; the native group bar starts its
    // OWN drag (dragController()->dragBegin directly, CHyprGroupBarDecoration.cpp:417) and returns true.
    bool decorationsTakeDragStart(SPT t) {
        if (t->foreignDecoStartReturnsFalse) {
            CallerScope g(*this, W_THIRD);
            if (!target()) dragBegin(t, M_MOVE); // the decoration itself checks target(); then returns false
            return false;
        }
        if (!t->groupBarStart)
            return false;
        SPT on = t;
        if (!t->groupBarStartsOn.empty())
            on = lookup(t->groupBarStartsOn);
        CallerScope g(*this, W_GROUPBAR);
        dragBegin(on, M_MOVE);
        if (auto f = onGroupBarFocus) f(); // copy: may clear itself; :419-420 rawWindowFocus -> window.active (synchronous)
        return true;
    }
    // ---- synchronous foreign code the host runs (event-bus listeners, decorations, layouts, Lua). Scenario-set. ----
    std::function<bool()> onDecoDragEnd;    // :287 checkInputOnDecorations(DRAG_END); true = decoration took it (:288-292)
    std::function<void()> onGroupAdd;       // :299 group->add (layout recalculation, focus inside the group)
    std::function<void()> onFloatingChange; // :314 changeFloatingMode -> window.floating (LayoutManager.cpp:52)
    std::function<void()> onSetTargetGeom;  // :321 setTargetGeom (layout)
    std::function<void()> onFocus;          // :323 fullWindowFocus -> window.active (FocusState.cpp:220)
    std::function<void()> onBeginFocus;     // :245 rawWindowFocus inside dragBegin -> window.active
    std::function<void()> onGroupBarFocus;  // CHyprGroupBarDecoration.cpp:420

    // ---- NOT in the 0.57 API: what a function hook on CDragStateController::dragBegin/dragEnd could count ----------
    // (one tick per dragBegin call and per dragEnd call that finds a live target). Used ONLY by PolHook, which models an
    // architectural alternative that needs hook + lifecycle authorization.
    uint64_t hookSeq() const { return m_hookSeq; }

    // test-only registry so a decoration can name another window
    std::vector<SPT> windows;
    SPT              lookup(const std::string& n) {
        for (auto& w : windows)
            if (w->name == n) return w;
        return nullptr;
    }

    // idle dispatch (CEventLoopManager::doLater -> wl_event_loop_add_idle). The real loop runs idle callbacks only from
    // wl_display_run (EventLoopManager.cpp:137), never from inside dragEnd or a signal emission; the model asserts it.
    std::vector<std::function<void()>> idleQueue;
    void                               runIdle() {
        if (m_endDepth || m_emitDepth) { std::printf("MODEL ERROR: idle inside dragEnd/emission\n"); std::abort(); }
        while (!idleQueue.empty()) {
            auto q = std::move(idleQueue);
            idleQueue.clear();
            for (auto& f : q) f();
        }
    }

    // caller attribution (truth)
    struct CallerScope {
        CHost& h;
        CallerScope(CHost& h_, eWho w) : h(h_) { h.m_callers.push_back(w); }
        ~CallerScope() { h.m_callers.pop_back(); }
    };

  private:
    // =========================== HOST STATE (private in 0.57) ===========================================
    WPT   m_target;
    eMode m_dragMode          = M_INVALID;
    bool  m_wasDraggingWindow = false;

    uint64_t m_hookSeq = 0;

    void dragBegin(SPT t, eMode mode) { // U:159-251
        ++m_hookSeq;
        noteBegin(t);
        m_target   = t; // :160 (no guard)
        m_dragMode = mode;
        const auto D = m_target.lock();
        if (!D->mapped) { dragEnd(); return; }        // :174-178
        if (!D->hasWorkspace) { dragEnd(); return; }  // :180-184
        if (D->fsWorkspaceReject) { dragEnd(); return; } // :189-190 -> updateDragWindow :123-127 (mapped path)
        if (auto f = onBeginFocus) f();               // :244-247 rawWindowFocus/raise
        markClean();
    }

    bool dragEnd() { // U:252-334
        auto D = m_target.lock();
        if (!D) return false; // :255-256: no `ended`
        ++m_hookSeq;
        noteEndEntry(D);
        ++m_endDepth;
        Hyprutils::Utils::CScopeGuard depth([this] { --m_endDepth; });                 // destroyed AFTER notifyEnded
        Hyprutils::Utils::CScopeGuard notifyEnded([this] { emitEnded(); });           // :258
        if (!D->mapped) {                                                              // :262-271
            if (D->hasWindow) resetTarget();                                           // :265
            m_dragMode = M_INVALID;
            return true;
        }
        resetTarget();                                                                 // :274
        m_wasDraggingWindow = true;                                                    // :275
        if (m_dragMode == M_MOVE && D->overOtherWindow) {                              // :277-286
            if (onDecoDragEnd) {
                auto f = onDecoDragEnd; // copy: a callback may clear itself
                if (f()) {                                                             // :287
                    m_wasDraggingWindow = false;                                       // :288
                    m_dragMode          = M_INVALID;                                   // :289
                    return true;                                                       // :292
                }
            }
            if (D->joinsGroup)
                if (auto f = onGroupAdd) f();                                          // :299
        }
        if (D->draggingTiled)
            if (auto f = onFloatingChange) f();                                        // :311-314
        if (auto f = onSetTargetGeom) f();                                             // :321
        if (auto f = onFocus) f();                                                     // :323
        m_wasDraggingWindow = false;                                                   // :325
        // :326-327 setClientResizingState -> XDG setResizing: only doLaterLock (XDGShell.cpp:444-498, 668-676);
        // X11Backend::setResizing is empty. No synchronous foreign code between :325 and the emission.
        m_dragMode = M_INVALID;                                                        // :329
        return true;
    }

    // =========================== TRUTH (compositor-internal / test-only) =================================
    friend struct Truth;
    std::vector<eWho>        m_callers;
    std::vector<SSession>    m_sessions;
    std::optional<size_t>    m_cur; // session whose target is m_target (valid only while m_target is alive)
    int                      m_endDepth = 0, m_emitDepth = 0;
    uint64_t                 m_emissions = 0;
    std::vector<std::string> m_violations;
    std::vector<SEndRequest> m_requests;

    eWho caller() const { return m_callers.empty() ? W_HOST : m_callers.back(); }
    void noteBegin(const SPT& t) {
        if (caller() == W_KWINISH && m_target.lock())
            m_violations.push_back("kwinish replaced an active session (unguarded begin)");
        SSession s;
        s.id     = m_sessions.size() + 1;
        s.who    = caller();
        s.target = t->name;
        for (auto& o : m_sessions)
            if (o.who == s.who && o.target == s.target) ++s.ordinal;
        s.cleanMark = UINT64_MAX;
        m_sessions.push_back(s);
        m_cur = m_sessions.size() - 1;
    }
    void markClean() { if (m_cur) m_sessions[*m_cur].cleanMark = m_emissions; }
    void noteEndEntry(const SPT&) {
        if (!m_cur) return;
        auto& s = m_sessions[*m_cur];
        if (caller() == W_KWINISH) {
            if (s.who != W_KWINISH)
                m_violations.push_back(std::string("kwinish ended a ") + whoName(s.who) + " session on " + s.target);
            s.endedByKwinish = true;
        } else
            s.endedByOther = true;
    }
    void resetTarget() { m_target.reset(); m_cur.reset(); }
    void emitEnded() {
        ++m_emissions;
        ++m_emitDepth;
        m_events.ended.emit();
        --m_emitDepth;
    }
};

// Oracle/driver access to the truth layer. Policies must not use this.
struct Truth {
    static std::optional<SSession> current(const CHost& h) {
        if (!h.m_target.lock() || !h.m_cur) return std::nullopt;
        return h.m_sessions[*h.m_cur];
    }
    static const std::vector<SSession>&    sessions(const CHost& h) { return h.m_sessions; }
    static const std::vector<std::string>& violations(const CHost& h) { return h.m_violations; }
    static std::vector<SEndRequest>&       requests(CHost& h) { return h.m_requests; }
    static int                             endDepth(const CHost& h) { return h.m_endDepth; }
    // the live session was begun by `who`, its begin completed (no refusal) and no `ended` was emitted since
    static bool startedCleanly(const CHost& h, eWho who) {
        const auto s = current(h);
        return s && s->who == who && s->cleanMark != UINT64_MAX && s->cleanMark == h.m_emissions;
    }
    static int                             emitDepth(const CHost& h) { return h.m_emitDepth; }
    // A Kwinish end entry is "provably safe" when: no dragEnd frame and no `ended` emission is on the stack, the live
    // session was begun by Kwinish itself and no `ended` was emitted since that begin returned. Only then is ending
    // REQUIRED (liveness). Everywhere else the policy may refuse.
    static void noteEndRequest(CHost& h, const std::string& where) {
        SEndRequest r;
        r.endDepth  = h.m_endDepth;
        r.emitDepth = h.m_emitDepth;
        r.where     = where;
        if (h.m_target.lock() && h.m_cur) {
            r.session          = *h.m_cur;
            const auto& s      = h.m_sessions[*h.m_cur];
            r.livenessRequired = !r.endDepth && !r.emitDepth && s.who == W_KWINISH && s.cleanMark == h.m_emissions;
        }
        h.m_requests.push_back(r);
    }
};
