// Q4-A host-free model (scratch only; NOT production code).
// - CDragModel replicates the control flow of the 0.57 CDragStateController::dragBegin/dragEnd
//   (U:layout/supplementary/DragController.cpp:158-251 and 252-334, commit 0aa6745f) at the level that matters for
//   ownership: who sets/resets m_target, where `ended` is emitted (scope guard after the early "no target" return),
//   the refusal paths inside dragBegin (unmapped window / no workspace -> dragEnd; fullscreen-workspace reject in
//   updateDragWindow -> dragEnd), the DRAG_END decoration early return, and the synchronous callbacks that run before
//   `ended` (decoration DRAG_END, fullWindowFocus -> window.active listeners).
// - Signals are the REAL Hyprutils::Signal::CSignalT<> / CHyprSignalListener (installed hyprutils 0.14.3).
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

struct STarget {
    std::string name;
    bool        mapped = true, hasWorkspace = true, hasWindow = true, fsWorkspaceReject = false, decoTakesDragEnd = false;
};
using SPT = CSharedPointer<STarget>;
using WPT = CWeakPointer<STarget>;

inline std::vector<std::string> g_log;
inline void                     L(const std::string& s) { g_log.push_back(s); }

struct CDragModel {
    WPT   m_target;
    eMode m_mode = M_INVALID;
    struct {
        CSignalT<> ended;
    } m_events;
    std::function<void()> onFocusDuringEnd;     // stands for fullWindowFocus -> window.active listeners
    std::function<void()> onDecoDragEnd;        // stands for checkInputOnDecorations(INPUT_TYPE_DRAG_END) callbacks
    int                   endedEmissions = 0;

    SPT target() const { return m_target.lock(); }

    void dragBegin(SPT t, eMode mode) { // U:158-251
        m_target = t;                    // :160  (no guard: callers must check target() first)
        m_mode   = mode;
        const auto D = m_target.lock();
        if (!D->mapped) { L("dragBegin: invalid window -> dragEnd"); dragEnd(); return; }        // :173-177
        if (!D->hasWorkspace) { L("dragBegin: no workspace -> dragEnd"); dragEnd(); return; }     // :179-183
        if (D->fsWorkspaceReject) { L("dragBegin: fullscreen workspace reject -> dragEnd"); dragEnd(); return; } // updateDragWindow :123-126
        L("dragBegin: started " + D->name);
    }

    bool dragEnd() { // U:252-334
        auto D = m_target.lock();
        if (!D)
            return false;                                                              // :255-256, no `ended`
        Hyprutils::Utils::CScopeGuard notifyEnded([this] { ++endedEmissions; m_events.ended.emit(); }); // :258
        if (!D->mapped) {                                                              // :262-271
            if (D->hasWindow)
                m_target.reset();
            m_mode = M_INVALID;
            return true;
        }
        m_target.reset();                                                              // :274 (before callbacks)
        if (m_mode == M_MOVE && D->decoTakesDragEnd) {                                 // :287-293
            if (auto f = onDecoDragEnd) f(); // copy: a callback may clear itself
            m_mode = M_INVALID;
            return true;
        }
        if (auto f = onDecoDragEnd) f();
        if (auto f = onFocusDuringEnd) f();                                      // :324 fullWindowFocus
        m_mode = M_INVALID;
        return true;
    }
};
inline CDragModel* g_dc = nullptr; // the host's single controller (g_layoutManager->dragController())

// ---- the plugin side ---------------------------------------------------------------------------------------------
// Start helper = plugin/hostDrag.hpp KwDrag::begin semantics (guard: refuse if any drag is active).
enum eBeginResult { STARTED, ALREADY, REFUSED };

// Candidate end policies WITHOUT ownership (what K3 could do today) -------------------------------------------------
struct CNaive {
    SPT  remembered;
    int  kind = 0; // 0 unconditional, 1 target equality, 2 target+mode
    eMode rememberedMode = M_INVALID;
    eBeginResult begin(SPT t, eMode m) {
        if (g_dc->target()) return ALREADY;
        g_dc->dragBegin(t, m);
        remembered = t; rememberedMode = m;
        return g_dc->target() ? STARTED : REFUSED;
    }
    bool endIfOurs() {
        const auto T = g_dc->target();
        if (!T) return false;
        if (kind == 1 && T != remembered) return false;
        if (kind == 2 && (T != remembered || g_dc->m_mode != rememberedMode)) return false;
        return g_dc->dragEnd();
    }
    bool owns() const { return true; }
};

// PROPOSED contract (Q4-A) -------------------------------------------------------------------------------------------
// Ownership = (a listener on `ended` armed only for our session) AND (no `ended` since acquisition) AND (our target
// still alive) AND (it is the controller's current target). Acquired only when our begin really started a session.
struct COwnership {
    uint64_t            generation = 0;     // bumped by accepted `ended` callbacks
    uint64_t            acquisition = 0;    // listener identity; NOT a host session ID
    bool                inBegin = false;   // reject plugin reentry during acquisition
    std::optional<uint64_t> ownedAt;        // generation at acquisition; nullopt = not owning
    WPT                 ownedTarget;
    CHyprSignalListener listener;           // session-scoped: exists only while owning (or while starting)
    bool                inOwnEnd = false;   // we are inside our own endDragTarget()
    bool                unloading = false;
    bool                failListen = false; // test hook: listen() throws (allocation failure)
    int                 handlerRuns = 0;

    void disarm() { ++acquisition; ownedAt.reset(); ownedTarget.reset(); listener.reset(); }

    bool owns() const { return ownedAt && *ownedAt == generation && !ownedTarget.expired() && g_dc->target() == ownedTarget.lock() && listener; }

    eBeginResult begin(SPT t, eMode m) {
        if (unloading || inOwnEnd || inBegin)       // acquire/end/unload are non-reentrant plugin transactions
            return REFUSED;
        if (g_dc->target())
            return ALREADY;                       // anyone's drag: untouched (P8-B guard)
        inBegin = true;
        Hyprutils::Utils::CScopeGuard finishBegin([this] { inBegin = false; });
        const uint64_t token = ++acquisition;
        try {                                     // arm BEFORE begin, so a refusal-`ended` inside dragBegin is seen
            if (failListen) throw std::bad_alloc();
            listener = g_dc->m_events.ended.listen([this, token] {
                // Old callbacks can survive in MORE THAN ONE signal snapshot; reference counts do not identify them.
                if (token != acquisition) return;
                ++handlerRuns;
                ++generation;
                disarm();
            });
        } catch (const std::bad_alloc&) {
            // rollback policy: start WITHOUT ownership (preserves today's start behaviour; the plugin will not end it,
            // the host ends it on the next button/key)
            g_dc->dragBegin(t, m);
            return g_dc->target() ? STARTED : REFUSED;
        }
        const uint64_t G0 = generation;
        g_dc->dragBegin(t, m);
        if (generation != G0 || !g_dc->target() || g_dc->target() != t) { // refused inside dragBegin (ended fired) or replaced
            disarm();
            return REFUSED;
        }
        ownedAt     = G0;
        ownedTarget = t;
        return STARTED;
    }

    bool endIfOurs() { // K3/K9 replacement
        if (!owns())
            return false;
        inOwnEnd       = true;
        const bool R   = g_dc->dragEnd(); // emits `ended` -> our handler disarms
        inOwnEnd       = false;
        if (ownedAt) disarm();            // e.g. host emitted nothing (cannot happen with a live target; defensive)
        return R;
    }

    void unloadStep2() { // PLUGIN_EXIT step 2 (Q4), before removing listeners/decorations
        unloading = true;
        if (owns())
            (void)endIfOurs();
        disarm();        // listener destroyed before the module is unmapped
    }
};
