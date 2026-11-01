// Q4-A1: "alternative 5" study - Kwinish never requests a drag end it cannot prove; the host ends drags on its own
// (every uncancelled button/key event). Host-free; NOT production code.
// Reuses the drag-controller model of the previous study unchanged (../Q4A-policy-tests/host.hpp, policies.hpp) and adds
// an input layer replicating U:managers/input/InputManager.cpp:720-773, 845-939 and U:keybinds/Manager.cpp:189-224,
// 345-417 at the level that matters here: bus listeners first (may cancel), release-without-held-press path, the
// keybind manager ending any drag on every non-captured button/key event, and forwarding to the client.
// Plugin side replicates the CURRENT event routes of plugin/barDeco.cpp and plugin/main.cpp (listed per site below).
#include "../Q4A-policy-tests/policies.hpp"

#include <cstring>
#include <map>
#include <memory>
#include <set>

enum eVariant { V_ABSENT, V_CUR, V_VETO, V_NAIVE5, V_ALT5, V_ALT5RC, V_COUNT };
static const char* VN[] = {"absent", "cur-0.56", "veto-rc", "naive5", "alt5", "alt5-rc"};
enum eWhere { AT_BAR, AT_BODY, AT_ELSEWHERE };
constexpr int BTN_L = 1, BTN_R = 3, BTN_M = 2; // BTN_LEFT / BTN_RIGHT / BTN_MIDDLE

struct SInfo {
    bool cancelled = false;
};

struct World;
// ---------------------------------------------------------------------------------------------------------------------
// One Kwinish title bar (CHyprBar) on one window.
struct Bar {
    SPT  win;
    bool dragging = false, cancelledDown = false, pending = false, rearmPending = false;
    bool snapped = false, unsnapChecked = false, unsnapAnim = false, deferredUnsnap = false;
    int  unsnapSteps = 0;
};

struct World {
    CHost                      h;
    eVariant                   v;
    std::unique_ptr<PolVeto>   veto; // V_VETO only (recheck variant)
    std::vector<Bar>           bars;
    bool                       grabActive = false, grabSwallowPress = false, unloaded = false;
    bool                       focusMoved = false;        // focus left the bar's window between press and release
    bool                       passMouseWhenBound = false; // binds:pass_mouse_when_bound
    using Btn = std::pair<int, int>;                      // (pointer, button): the host tracks both (InputManager.cpp:746-753)
    std::set<Btn>              held;                      // CInputManager::m_currentlyHeldButtons
    std::set<Btn>              clientPressed;             // what the focused client was told
    std::set<Btn>              hostSwallowedPress, hostSwallowedRelease; // host quirk: DRAG_WAS_ACTIVE (Manager.cpp:388/416)
    bool                       captured     = false;      // input capture active (InputManager.cpp:736-741)
    bool                       bindsAllowed = true;       // keyboard->m_allowBinds (Manager.cpp:197-198)
    bool                       keyReachesDragEnd = true;  // false: DISALLOWACTION / translation fail :203 / internal keybind :215 (review 2)
    std::vector<std::string>   o2;
    std::set<uint64_t>         blocked; // sessions alive while the plugin cancelled a button event the host would see
    std::vector<std::string>   o3, spec, notes;
    int                        endRequests = 0;
    std::function<bool(int)>   userBind; // host bind on press (e.g. mod+LMB movewindow): returns true if it consumed
    explicit World(eVariant v_) : v(v_) {
        if (v == V_VETO) veto = std::make_unique<PolVetoRecheck>(h);
    }
    SPT mk(const char* n) {
        auto t  = makeShared<STarget>();
        t->name = n;
        h.windows.push_back(t);
        return t;
    }
    Bar& bar(SPT w) {
        bars.push_back({});
        bars.back().win = w;
        return bars.back();
    }

    // ---- plugin -> host requests --------------------------------------------------------------------------------
    eBeginResult kwBegin(SPT t, eMode m = M_MOVE) { // KwDrag::begin (K1/K2)
        CHost::CallerScope s(h, W_KWINISH);
        switch (v) {
            case V_VETO: return veto->begin(t, m);
            case V_ALT5RC: return kwDragBeginRechecked(h, t, m);
            default: return kwDragBegin(h, t, m);
        }
    }
    // K3/K9 sites (changeMouseBindMode(MBIND_INVALID) / dispatcher "mouse 0movewindow"): 0.56.2 ended ANY drag.
    void kwEnd(const char* where) {
        ++endRequests;
        CHost::CallerScope s(h, W_KWINISH);
        switch (v) {
            case V_CUR: (void)h.endDragTarget(); break;
            case V_VETO: veto->requestEnd(); break;
            default: notes.push_back(std::string("end request dropped at ") + where); break; // alternative 5
        }
    }
    void kwRearm(Bar& b) { // barDeco.cpp:273-280
        CHost::CallerScope s(h, W_KWINISH);
        switch (v) {
            case V_CUR: (void)h.endDragTarget(); (void)kwDragBegin(h, b.win, M_MOVE); break;
            case V_VETO: veto->requestRearm(b.win, M_MOVE); break;
            default: (void)kwBegin(b.win); break; // begin only: refuses (ALREADY) if any drag exists
        }
    }
    bool rewritesInput() const { return v == V_ALT5 || v == V_ALT5RC; }
    // Config::Actions::floatWindow (barDeco.cpp:337,380,652,696 via runAction "float"): ConfigActions.cpp:249-253 ends
    // ANY active drag when the floating state changes. An indirect Kwinish end request outside the K3/K9 inventory.
    void kwFloatAction(SPT w, bool wantFloat) {
        if (v == V_ABSENT) return;
        if (rewritesInput() && h.target()) { // alt 5 guard: do not toggle (and thus end) while any drag is active
            notes.push_back("float toggle refused while a drag is active");
            return;
        }
        const bool isFloat = !w->draggingTiled;
        if (wantFloat == isFloat) return;
        { CHost::CallerScope s(h, W_KWINISH); if (h.target()) (void)h.endDragTarget(); }
        w->draggingTiled = !wantFloat;
    }

    // ---- plugin listeners (event bus) -------------------------------------------------------------------------------
    // main.cpp:475-490 (PG), registered in pluginInit BEFORE the bars (decorations are added at main.cpp:514)
    void pgButton(bool pressed, SInfo& info) {
        if (pressed && grabActive && rewritesInput() && !h.target()) { // alt 5: the grab's drag is already gone
            grabActive = false;
            notes.push_back("grab click with no drag: passed through");
            return;
        }
        if (pressed && grabActive) {
            grabActive       = false;
            grabSwallowPress = true;
            kwEnd("main.cpp:484 grab click");
            info.cancelled = true; // the click must not reach the app (both designs)
        } else if (!pressed && grabSwallowPress) {
            grabSwallowPress = false;
            if (!rewritesInput()) info.cancelled = true; // alt 5: let the release reach the host, which ends the drag
        }
    }
    void handleMovement(Bar& b) { // barDeco.cpp:664-681
        b.unsnapChecked = false;
        b.unsnapAnim    = false;
        if (rewritesInput() && b.snapped) { // alt 5 redesign: unsnap BEFORE any host drag, begin afterwards
            b.deferredUnsnap = true;
            b.dragging       = true;
            return;
        }
        (void)kwBegin(b.win);
        b.dragging = true; // production sets it regardless of the begin result
    }
    void handleUp(Bar& b, SInfo& info) { // barDeco.cpp:625-662
        if (focusMoved) {
            if (b.dragging) kwEnd("barDeco.cpp:630 focus moved");
            b.dragging = b.cancelledDown = b.pending = false;
            return; // NOT cancelled: the host sees this release
        }
        if (b.cancelledDown && !rewritesInput()) info.cancelled = true; // :639-640
        b.cancelledDown = false;
        if (rewritesInput()) b.rearmPending = false; // alt 5 (review 2): the interaction is over
        if (b.dragging) {
            if (b.unsnapAnim || b.deferredUnsnap) { b.unsnapAnim = b.deferredUnsnap = false; b.unsnapSteps = 0; } // land on final size
            kwEnd("barDeco.cpp:649 release");
            b.dragging = false;
            if (rewritesInput() && h.target()) notes.push_back("snapOnRelease runs BEFORE the host ends the drag (ordering change)");
        }
        b.pending = false;
    }
    void barButton(Bar& b, int btn, bool pressed, eWhere where, SInfo& info) { // barDeco.cpp:154-197
        if (grabActive || grabSwallowPress) return;
        if (btn != BTN_L && btn != BTN_R) return; // :160-161
        if (pressed && info.cancelled) return;
        if (btn == BTN_R) { // :169-193: the release is ignored; a press on the bar opens the menu and is cancelled
            if (!pressed || where != AT_BAR) return;
            if (rewritesInput() && h.target()) { // alt 5 (review 2): yield to an active drag, no menu
                notes.push_back("bar right-click yields to an active drag");
                return;
            }
            info.cancelled = true; // :190
            return;
        }
        if (!pressed && (b.dragging || b.cancelledDown || b.pending)) { handleUp(b, info); return; }
        if (where == AT_ELSEWHERE) return; // inputIsValid()
        if (!pressed) { handleUp(b, info); return; }
        // handleDownEvent :347-427
        if (where != AT_BAR) { // :376-389 press outside the bar
            if (b.dragging) kwEnd("barDeco.cpp:381 K9");
            b.dragging = b.pending = false;
            return;
        }
        if (rewritesInput() && h.target()) { // alt 5: a drag is active (anyone's): leave the press to the host
            notes.push_back("bar press yields to an active drag");
            return;
        }
        info.cancelled  = true; // :397
        b.cancelledDown = true;
        if (b.win->draggingTiled) b.pending = true; // tiled/maximized: start on first motion
        else handleMovement(b);                     // floating: start at the press
    }
    void stepUnsnapAlt5(Bar& b) {
        if (h.target()) { // someone else is dragging: stop animating, never start ours on top of it
            b.deferredUnsnap = false;
            b.unsnapSteps    = 0;
            notes.push_back("unsnap aborted: another drag is active");
            return;
        }
        if (h.target()) spec.push_back("unsnap animates while another drag is active");
        if (--b.unsnapSteps > 0) return;
        b.deferredUnsnap = false;
        b.snapped        = false;
        (void)kwBegin(b.win); // begin only, after the animation
    }
    void barMotion(Bar& b) { // barDeco.cpp:282-325
        if (b.rearmPending) {
            b.rearmPending = false;
            if (b.dragging && !b.deferredUnsnap) kwRearm(b); // rearmDrag :273-280 (alt 5: the unsnap begins the drag itself)
        }
        if (b.deferredUnsnap) {
            if (!b.unsnapSteps) b.unsnapSteps = 2;
            stepUnsnapAlt5(b);
            return;
        }
        if (b.dragging && !rewritesInput()) { // maybeUnsnap / stepUnsnap :540-589 (alt 5 replaces it before the begin)
            if (b.snapped && !b.unsnapChecked) {
                b.unsnapChecked = true;
                kwEnd("barDeco.cpp:566 unsnap pause");
                b.unsnapAnim  = true;
                b.unsnapSteps = 2;
            }
            if (b.unsnapAnim) {
                if (h.target() && h.target() == b.win) spec.push_back("unsnap animates while a host drag moves the same window");
                if (--b.unsnapSteps == 0) {
                    b.unsnapAnim = false;
                    b.snapped    = false;
                    kwEnd("barDeco.cpp:586 unsnap restart");
                    (void)kwBegin(b.win);
                }
            }
        }
        if (b.pending) { b.pending = false; handleMovement(b); }
    }

    // ---- host input (InputManager / KeybindManager) ------------------------------------------------------------------
    bool keybindMouse(bool captured = false) { // Manager.cpp:353: every non-captured button event ends the drag
        CHost::CallerScope s(h, W_HOST);
        return captured ? false : h.endDragTarget();
    }
    bool idleInteraction() const {
        if (grabActive || grabSwallowPress || !held.empty()) return false;
        for (auto& b : bars)
            if (b.dragging || b.pending || b.cancelledDown || b.unsnapAnim || b.deferredUnsnap) return false;
        return true;
    }
    void checkInteractionEnd() { // O2 at every point where the interaction is over, not only at the end
        if (!idleInteraction()) return;
        if (auto s = Truth::current(h); s && s->who == W_KWINISH) o2.push_back("Kwinish drag on " + s->target + " alive after its interaction ended");
    }
    void button(int btn, bool pressed, eWhere where, int ptr = 1) { // InputManager.cpp:720-773
        const Btn id{ptr, btn};
        button_(id, pressed, where);
        if (!pressed) checkInteractionEnd();
    }
    void button_(Btn id, bool pressed, eWhere where) {
        SInfo      info;
        const auto before = Truth::current(h); // only a drag already active when the event arrives can be ended by it
        if (v != V_ABSENT && !unloaded) {
            pgButton(pressed, info);
            for (auto& b : bars) barButton(b, id.second, pressed, where, info);
        }
        if (info.cancelled) {
            if (auto s = Truth::current(h); s && before && before->id == s->id) {
                blocked.insert(s->id); // this event would have ended it (Manager.cpp:353) unless captured
                if (s->who != W_KWINISH && !captured)
                    o3.push_back(std::string("cancelled an event that natively ends the ") + whoName(s->who) + " drag on " + s->target);
            }
            return;
        }
        if (captured) { // :736-741: onMouseEvent(captured=true) does NOT end the drag; nothing reaches the client
            (void)keybindMouse(true);
            if (!pressed) held.erase(id);
            return;
        }
        if (!pressed && !held.contains(id)) { (void)keybindMouse(); return; } // :748-751, never forwarded
        if (pressed) held.insert(id); else held.erase(id);
        const bool dragWas = keybindMouse(); // processMouseDownNormal :849
        bool       bound   = false;
        if (pressed && userBind) { CHost::CallerScope s(h, W_USER); bound = userBind(id.second); }
        const bool pass = !bound && !dragWas;
        if (!pass && !passMouseWhenBound) { // :857-858
            if (dragWas) (pressed ? hostSwallowedPress : hostSwallowedRelease).insert(id);
            return;
        }
        // :928 sendPointerButton
        if (pressed) clientPressed.insert(id);
        else if (!clientPressed.erase(id)) {
            if (hostSwallowedPress.contains(id)) notes.push_back("host quirk: press swallowed because a drag was active, release forwarded");
            else o3.push_back("client received a release without its press");
        }
    }
    void motion() { // InputManager.cpp:292 bus mouse.move, then the drag controller moves the target
        if (v != V_ABSENT && !unloaded)
            for (auto& b : bars) barMotion(b);
    }
    void key(bool shift) { // InputManager.cpp:1712 bus key, then Manager.cpp:223 ends any drag
        if (v != V_ABSENT && !unloaded) {
            if (!shift && grabActive) grabActive = false; // main.cpp:459-460 (Escape)
            if (rewritesInput() && grabActive && bindsAllowed) { // alt 5: the host ends the grab drag on any key event
                grabActive = false;
                notes.push_back("grab flag cleared: the host ended the drag on a key event");
            }
            if (shift)
                for (auto& b : bars)
                    if (!rewritesInput() || b.dragging) b.rearmPending = true; // main.cpp:463-466 (alt 5: only a bar mid-drag)
        }
        if (!bindsAllowed || !keyReachesDragEnd) return; // Manager.cpp:197-198, :203, :215; InputManager DISALLOWACTION
        CHost::CallerScope s(h, W_HOST);
        (void)h.endDragTarget();
    }
    void luaGrab(SPT w) { // main.cpp:302-320 (K2)
        if (v == V_ABSENT) return;
        const auto r = kwBegin(w);
        if (rewritesInput() && r != STARTED) { // alt 5 (review 2): no grab state without an acquired drag
            notes.push_back("grab not armed: begin did not start");
            return;
        }
        grabActive = true;
    }
    void unload() { // PLUGIN_EXIT today ends nothing; veto-rc would end only proven own drags
        if (v == V_VETO) { CHost::CallerScope s(h, W_KWINISH); veto->unload(); }
        unloaded = true;
        bars.clear();
        grabActive = grabSwallowPress = false;
    }
};

// ---------------------------------------------------------------------------------------------------------------------
struct SScenario {
    const char*                name;
    const char*                what;
    std::function<void(World&)> body;
};
struct SResult {
    std::vector<std::string> o1, o2, o3, o4, spec, info, host;
    bool                     hasDrag = false;
    std::string              fin;
};

static SResult run(eVariant v, const SScenario& sc) {
    World w(v);
    sc.body(w);
    w.h.runIdle();
    SResult r;
    r.o1 = Truth::violations(w.h);
    r.o2 = w.o2;
    // O2: every Kwinish-started session is finished by the end of the scenario (all buttons released)
    if (auto s = Truth::current(w.h); s && s->who == W_KWINISH) r.o2.push_back("Kwinish drag on " + s->target + " still active after the interaction");
    // O3: input not blocked: no stale held button, client pairing (collected), and the plugin did not keep alive a
    // drag that the native host (no plugin) would have ended
    r.o3 = w.o3;
    if (!w.held.empty()) r.o3.push_back("host keeps a released button as held");
    for (const auto& id : w.clientPressed) {
        if (w.hostSwallowedRelease.contains(id)) w.notes.push_back("host quirk: release swallowed because a drag was active (client keeps the press)");
        else r.o3.push_back("client still thinks a button is pressed");
    }
    if (auto s = Truth::current(w.h); s && w.blocked.contains(s->id))
        r.o3.push_back(std::string("release/click cancelled by Kwinish while this drag was active; still active: ") + whoName(s->who) + ":" + s->target);
    // O4: final consistency between host and plugin state
    const auto cur = Truth::current(w.h);
    r.hasDrag      = cur.has_value();
    r.fin          = cur ? std::string(whoName(cur->who)) + ":" + cur->target + " mode=" + std::to_string(w.h.mode()) : "no drag";
    if (w.h.wasDraggingWindow()) r.o4.push_back("wasDraggingWindow left set");
    if (!cur && w.h.mode() != M_INVALID) r.o4.push_back("mode set without a drag");
    for (auto& b : w.bars)
        if (b.dragging || b.pending || b.cancelledDown || b.unsnapAnim || b.deferredUnsnap) r.o4.push_back("bar flags left set after the interaction");
    if (w.grabActive || w.grabSwallowPress) r.o4.push_back("grab flags left set");
    for (auto& b : w.bars)
        if (b.rearmPending) r.o4.push_back("rearm flag left set after the interaction (review 2)");
    for (auto& b : w.bars)
        if (b.unsnapSteps) r.o4.push_back("unsnap step counter left set");
    if (cur && w.h.mode() == M_INVALID) r.host.push_back(std::string("host-caused: live ") + whoName(cur->who) + " drag with mode INVALID (enclosing dragEnd, R3)");
    r.spec = w.spec;
    r.info = w.notes;
    return r;
}

static void expectKw(World& w, const char* what) { // was a local closure captured by reference (review 1: dangling)
    if (w.v == V_ABSENT) return;                     // no plugin: nothing to resume
    auto s = Truth::current(w.h);
    if (!s || s->who != W_KWINISH) w.spec.push_back(what);
}

static std::vector<SScenario> scenarios() {
    std::vector<SScenario> v;
    auto add = [&](const char* n, const char* w, std::function<void(World&)> b) { v.push_back({n, w, std::move(b)}); };

    // ---- bar drags ----
    add("B1.bar.floating", "press on bar (floating: begin at press), move, release", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); expectKw(w, "own drag active while moving"); w.button(1, false, AT_BAR);
    });
    add("B2.bar.tiled", "press on bar (tiled: begin on first motion), move, release", [&](World& w) {
        auto A = w.mk("A"); A->draggingTiled = true; w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); expectKw(w, "own drag active while moving"); w.motion(); w.button(1, false, AT_BAR);
    });
    add("B3.bar.click", "click on bar without motion (floating drag begun at press)", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.button(1, false, AT_BAR);
    });
    add("B4.bar.focus-moved", "focus moves between press and release", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.focusMoved = true; w.button(1, false, AT_ELSEWHERE);
    });
    add("B5.bar.K9-second-press", "second pointer presses on the window body while dragging", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.button(1, true, AT_BODY, 2); w.button(1, false, AT_BODY, 2); w.button(1, false, AT_BAR);
    });
    // ---- natural host end ----
    add("N1.host.key-end", "Escape mid-drag (host ends), keep moving, release", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.key(false); w.motion(); w.button(1, false, AT_BAR);
    });
    add("N2.host.unmap", "dragged window is closed mid-drag (host ends), release", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion();
        { CHost::CallerScope s(w.h, W_HOST); w.h.endDragTarget(); }
        w.button(1, false, AT_ELSEWHERE);
    });
    // ---- group bar ----
    add("G1.groupbar.via-kwinish-begin", "press on the Kwinish bar of a grouped window: the group bar takes DRAG_START", [&](World& w) {
        auto A = w.mk("A"); A->groupBarStart = true; w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.button(1, false, AT_BAR);
    });
    add("G2.groupbar.user-tab", "user drags a tab from the native group bar (host bind)", [&](World& w) {
        auto A = w.mk("A"); A->groupBarStart = true; w.bar(A);
        w.userBind = [&w, A](int) { if (!w.h.target()) return w.h.decorationsTakeDragStart(A); return false; };
        w.button(1, true, AT_ELSEWHERE); w.motion(); w.button(1, false, AT_ELSEWHERE);
    });
    add("G3.groupbar.drop-into-group", "own drag released over a group: group add + focus inside the host end", [&](World& w) {
        auto A = w.mk("A"); A->joinsGroup = true; w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.button(1, false, AT_BAR);
    });
    // ---- menu grab ----
    add("M1.grab.click", "window menu Move: grab, move, click ends it", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.luaGrab(A); w.motion(); w.button(1, true, AT_ELSEWHERE);
        if (Truth::current(w.h)) w.notes.push_back("grab still moving the window between the click press and its release (ends at release)");
        w.button(1, false, AT_ELSEWHERE);
    });
    add("M2.grab.escape", "grab, Escape (host ends), then a normal click", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.luaGrab(A); w.key(false); w.button(1, true, AT_ELSEWHERE); w.button(1, false, AT_ELSEWHERE);
    });
    add("M3.grab.third-after-escape", "grab, Escape, third party starts a drag, then a click", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.luaGrab(A); w.key(false);
        { CHost::CallerScope s(w.h, W_THIRD); if (!w.h.target()) w.h.beginDragTarget(A, M_MOVE); }
        w.button(1, true, AT_ELSEWHERE); w.button(1, false, AT_ELSEWHERE);
    });
    // ---- rearm (Shift) ----
    add("RA1.rearm.shift", "Shift mid-drag (host ends on every key event), motion rearms, release", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.key(true); w.motion(); expectKw(w, "drag resumed after Shift press");
        w.key(true); w.motion(); expectKw(w, "drag resumed after Shift release"); w.button(1, false, AT_BAR);
    });
    add("RA2.rearm.third", "Shift ends own drag; a third party starts one on the same window; motion would rearm", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.key(true);
        { CHost::CallerScope s(w.h, W_THIRD); if (!w.h.target()) w.h.beginDragTarget(A, M_RESIZE); }
        w.motion(); w.button(1, false, AT_BAR);
    });
    // ---- unsnap ----
    add("US1.unsnap", "drag a snapped window: size animates, then the move continues", [&](World& w) {
        auto A = w.mk("A"); auto& b = w.bar(A); b.snapped = true;
        w.button(1, true, AT_BAR); w.motion(); w.motion(); w.motion(); expectKw(w, "move continues after the unsnap animation");
        if (w.v != V_ABSENT && w.bars[0].snapped) w.spec.push_back("window was not unsnapped");
        w.button(1, false, AT_BAR);
    });
    add("US2.unsnap.release-mid", "release during the unsnap animation", [&](World& w) {
        auto A = w.mk("A"); auto& b = w.bar(A); b.snapped = true;
        w.button(1, true, AT_BAR); w.motion(); w.button(1, false, AT_BAR);
    });
    add("US3.unsnap.third", "third party starts a drag during the unsnap animation", [&](World& w) {
        auto A = w.mk("A"); auto& b = w.bar(A); b.snapped = true;
        w.button(1, true, AT_BAR); w.motion();
        { CHost::CallerScope s(w.h, W_THIRD); if (!w.h.target()) w.h.beginDragTarget(A, M_RESIZE); }
        w.motion(); w.motion(); w.button(1, false, AT_BAR);
    });
    // ---- third parties / previous regressions in input form ----
    add("T1.third.same-window", "host ends own drag (key); third party drags the same window; bar release", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.key(false);
        { CHost::CallerScope s(w.h, W_THIRD); if (!w.h.target()) w.h.beginDragTarget(A, M_MOVE); }
        w.button(1, false, AT_BAR);
    });
    add("R4i.rebegin-in-host-end", "bar release; inside the host end a third party re-begins the same window", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion();
        auto once = std::make_shared<bool>(false);
        w.h.onFocus = [&w, A, once] { if (*once) return; *once = true; CHost::CallerScope s(w.h, W_THIRD); if (!w.h.target()) w.h.beginDragTarget(A, M_MOVE); };
        w.button(1, false, AT_BAR);
    });
    add("H1i.deco-start-false", "Kwinish press: a third-party DRAG_START decoration starts a drag and returns false", [&](World& w) {
        auto A = w.mk("A"); A->foreignDecoStartReturnsFalse = true; w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.button(1, false, AT_BAR);
    });
    add("L1i.unguarded-third", "third party replaces the own drag without a guard (A3 violation); bar release", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion();
        { CHost::CallerScope s(w.h, W_THIRD); w.h.beginDragTarget(A, M_RESIZE); }
        w.button(1, false, AT_BAR);
    });
    add("F1.float-during-third", "bar 'float' button / touch tiling while a third-party drag is active", [&](World& w) {
        auto A = w.mk("A"), B = w.mk("B"); w.bar(A);
        { CHost::CallerScope s(w.h, W_THIRD); w.h.beginDragTarget(B, M_MOVE); }
        w.kwFloatAction(A, false);
        { CHost::CallerScope s(w.h, W_THIRD); w.h.endDragTarget(); }
    });
    add("C1.capture-at-release", "input capture becomes active before the bar release (review 1)", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.captured = true; w.button(1, false, AT_BAR);
    });
    add("C2.capture-then-key", "capture active at the bar release; later a key press (Manager.cpp:223 ends drags even when captured)", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.captured = true; w.button(1, false, AT_BAR); w.key(false); w.checkInteractionEnd();
    });
    add("K1.grab.escape-nobinds-click", "grab; Escape on a keyboard with binds disabled; then a click", [&](World& w) {
        auto A = w.mk("A"); w.bar(A); w.bindsAllowed = false;
        w.luaGrab(A); w.key(false); w.motion(); w.button(1, true, AT_ELSEWHERE); w.button(1, false, AT_ELSEWHERE);
    });
    add("K2.grab.escape-nobinds", "grab; Escape on a keyboard with binds disabled; nothing else (review 1)", [&](World& w) {
        auto A = w.mk("A"); w.bar(A); w.bindsAllowed = false;
        w.luaGrab(A); w.key(false); w.motion(); w.checkInteractionEnd();
    });
    add("Y1.bar-press-during-third", "a third-party drag is active when the bar is pressed (review 1)", [&](World& w) {
        auto A = w.mk("A"), B = w.mk("B"); w.bar(A);
        { CHost::CallerScope s(w.h, W_THIRD); w.h.beginDragTarget(B, M_MOVE); }
        w.button(1, true, AT_BAR); w.motion(); w.button(1, false, AT_BAR);
    });
    add("M4.grab.shift-then-third", "grab; Shift (host ends it, grab flag stays); third-party drag; click", [&](World& w) {
        auto A = w.mk("A"), B = w.mk("B"); w.bar(A);
        w.luaGrab(A); w.key(true);
        { CHost::CallerScope s(w.h, W_THIRD); w.h.beginDragTarget(B, M_MOVE); }
        w.button(1, true, AT_ELSEWHERE); w.button(1, false, AT_ELSEWHERE);
    });
    add("US4.unsnap.shift-before-motion", "snapped bar press, Shift before the first motion, then move (review 1)", [&](World& w) {
        auto A = w.mk("A"); auto& b = w.bar(A); b.snapped = true;
        w.button(1, true, AT_BAR); w.key(true); w.motion(); w.motion(); w.motion();
        expectKw(w, "move continues after the unsnap animation");
        if (w.v != V_ABSENT && w.bars[0].snapped) w.spec.push_back("window was not unsnapped");
        w.button(1, false, AT_BAR);
    });
    add("R5.right-click-during-third", "a third-party drag is active; right press on a bar (menu route) (review 2)", [&](World& w) {
        auto A = w.mk("A"), B = w.mk("B"); w.bar(A);
        { CHost::CallerScope s(w.h, W_THIRD); w.h.beginDragTarget(B, M_MOVE); }
        w.button(BTN_R, true, AT_BAR); w.button(BTN_R, false, AT_BAR);
    });
    add("R6.middle-on-bar-during-own", "own bar drag; middle button pressed and released on the bar", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.button(BTN_M, true, AT_BAR); w.button(BTN_M, false, AT_BAR); w.button(1, false, AT_BAR);
    });
    add("M5.grab.fails-during-third", "grab requested while a third-party drag is active; then a click (review 2)", [&](World& w) {
        auto A = w.mk("A"), B = w.mk("B"); w.bar(A);
        { CHost::CallerScope s(w.h, W_THIRD); w.h.beginDragTarget(B, M_MOVE); }
        w.luaGrab(A); w.button(1, true, AT_ELSEWHERE); w.button(1, false, AT_ELSEWHERE);
    });
    add("K3.grab.key-not-ending", "grab; a key that never reaches Manager.cpp:223 (DISALLOWACTION/internal bind); nothing else (review 2)", [&](World& w) {
        auto A = w.mk("A"); w.bar(A); w.keyReachesDragEnd = false;
        w.luaGrab(A); w.key(false); w.motion(); w.checkInteractionEnd();
    });
    add("K4.grab.key-not-ending-click", "as K3, then a click", [&](World& w) {
        auto A = w.mk("A"); w.bar(A); w.keyReachesDragEnd = false;
        w.luaGrab(A); w.key(false); w.motion(); w.button(1, true, AT_ELSEWHERE); w.button(1, false, AT_ELSEWHERE);
    });
    add("RA3.shift-then-release", "Shift mid-drag, release before any motion (rearm flag) (review 2)", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.key(true); w.button(1, false, AT_BAR);
    });
    add("U1.unload.mid-drag", "plugin unloaded mid-drag; the release arrives afterwards", [&](World& w) {
        auto A = w.mk("A"); w.bar(A);
        w.button(1, true, AT_BAR); w.motion(); w.unload(); w.button(1, false, AT_BAR);
    });
    add("P1.pass-mouse-when-bound", "binds:pass_mouse_when_bound=1, bar drag", [&](World& w) {
        auto A = w.mk("A"); w.bar(A); w.passMouseWhenBound = true;
        w.button(1, true, AT_BAR); w.motion(); w.button(1, false, AT_BAR);
    });
    add("P2.pass-mouse-grab", "binds:pass_mouse_when_bound=1, menu grab click", [&](World& w) {
        auto A = w.mk("A"); w.bar(A); w.passMouseWhenBound = true;
        w.luaGrab(A); w.motion(); w.button(1, true, AT_ELSEWHERE); w.button(1, false, AT_ELSEWHERE);
    });
    return v;
}

int main(int argc, char** argv) {
    const bool verbose = argc > 1 && !std::strcmp(argv[1], "-v");
    const auto sc      = scenarios();
    std::map<std::pair<std::string, int>, SResult> R;
    for (auto& s : sc)
        for (int v = 0; v < V_COUNT; ++v) R[{s.name, v}] = run((eVariant)v, s);
    std::printf("%-30s", "scenario \\ variant");
    for (int v = 0; v < V_COUNT; ++v) std::printf(" %-9s", VN[v]);
    std::printf("\n");
    for (auto& s : sc) {
        std::printf("%-30s", s.name);
        for (int v = 0; v < V_COUNT; ++v) {
            auto& r = R[{s.name, v}];
            std::string c = std::string(r.o1.empty() ? "." : "1") + (r.o2.empty() ? "." : "2") + (r.o3.empty() ? "." : "3") + (r.o4.empty() ? "." : "4") + (r.spec.empty() ? "" : "s") + (r.host.empty() ? "" : "h");
            std::printf(" %-9s", c.c_str());
        }
        std::printf("\n");
    }
    std::printf("legend: 1 = O1 Kwinish ended/replaced a foreign drag, 2 = O2 own drag not finished, 3 = O3 input blocked/stale, 4 = O4 inconsistent final state, s = scenario check, h = host-caused inconsistency (info)\n");
    for (auto& s : sc)
        for (int v = 0; v < V_COUNT; ++v) {
            auto& r = R[{s.name, v}];
            std::printf("RES %s %s final=[%s]\n", VN[v], s.name, r.fin.c_str());
            if (verbose || v == V_ALT5 || v == V_ALT5RC)
                for (auto* vec : {&r.o1, &r.o2, &r.o3, &r.o4, &r.spec, &r.host, &r.info})
                    for (auto& e : *vec) std::printf("    %s/%s: %s\n", VN[v], s.name, e.c_str());
        }
    return 0;
}
