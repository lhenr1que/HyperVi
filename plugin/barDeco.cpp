#include "barDeco.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/desktop/state/LayerState.hpp>
#include <hyprland/src/desktop/state/ViewHitTester.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/desktop/view/LayerSurface.hpp>
#include <hyprland/src/helpers/MiscFunctions.hpp>
#include <hyprland/src/managers/SeatManager.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/config/shared/animation/AnimationTree.hpp>
#include <hyprland/src/config/shared/parserUtils/ParserUtils.hpp>
#include <hyprland/src/config/supplementary/executor/Executor.hpp>
#include <hyprland/src/config/shared/actions/ConfigActions.hpp>
#include <hyprland/src/animation/AnimationManager.hpp>
#include <hyprland/src/protocols/LayerShell.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/layout/LayoutManager.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/state/MonitorState.hpp>

#include "globals.hpp"
#include "BarPassElement.hpp"
#include "icon.hpp"
#include "PreviewPassElement.hpp"

#include <climits>

using namespace Render::GL;

static CHyprColor configColor(Config::INTEGER color) {
    return CHyprColor{static_cast<uint64_t>(color)};
}

CHyprBar::CHyprBar(PHLWINDOW pWindow) : IHyprWindowDecoration(pWindow) {
    m_pWindow = pWindow;

    const auto PMONITOR         = pWindow->m_monitor.lock();
    PMONITOR->m_scheduledRecalc = true;

    // button events
    m_pMouseButtonCallback = Event::bus()->m_events.input.mouse.button.listen([&](IPointer::SButtonEvent e, Event::SCallbackInfo& info) { onMouseButton(info, e); });
    m_pTouchDownCallback   = Event::bus()->m_events.input.touch.down.listen([&](ITouch::SDownEvent e, Event::SCallbackInfo& info) { onTouchDown(info, e); });
    m_pTouchUpCallback     = Event::bus()->m_events.input.touch.up.listen([&](ITouch::SUpEvent e, Event::SCallbackInfo& info) { onTouchUp(info, e); });

    // move events
    m_pTouchMoveCallback = Event::bus()->m_events.input.touch.motion.listen([&](ITouch::SMotionEvent e, Event::SCallbackInfo& info) { onTouchMove(info, e); });
    m_pMouseAxisCallback = Event::bus()->m_events.input.mouse.axis.listen([&](IPointer::SAxisEvent e, Event::SCallbackInfo& info) { onMouseAxis(info, e); });
    m_pMouseMoveCallback = Event::bus()->m_events.input.mouse.move.listen([&](Vector2D c, Event::SCallbackInfo& info) { onMouseMove(c); });

    Animation::mgr()->createAnimation(configColor(g_pGlobalState->config.barColor->value()), m_cRealBarColor, Config::animationTree()->getAnimationPropertyConfig("border"),
                                      pWindow, AVARDAMAGE_NONE);
    m_cRealBarColor->setUpdateCallback([&](auto) { damageEntire(); });
}

CHyprBar::~CHyprBar() {
    std::erase(g_pGlobalState->bars, m_self);
}

SDecorationPositioningInfo CHyprBar::getPositioningInfo() {
    const auto                 HEIGHT     = g_pGlobalState->config.barHeight->value();
    const auto                 ENABLED    = g_pGlobalState->config.enabled->value();
    const auto                 PRECEDENCE = g_pGlobalState->config.barPrecedenceOverBorder->value();

    SDecorationPositioningInfo info;
    info.policy         = m_hidden ? DECORATION_POSITION_ABSOLUTE : DECORATION_POSITION_STICKY;
    info.edges          = DECORATION_EDGE_TOP;
    info.priority       = PRECEDENCE ? 10005 : 5000;
    info.reserved       = true;
    info.desiredExtents = {{0, !barActive() || !ENABLED ? 0 : HEIGHT}, {0, 0}};
    return info;
}

void CHyprBar::onPositioningReply(const SDecorationPositioningReply& reply) {
    if (reply.assignedGeometry.size() != m_bAssignedBox.size())
        m_bWindowSizeChanged = true;

    m_bAssignedBox = reply.assignedGeometry;
}

std::string CHyprBar::getDisplayName() {
    return "KwinishBar";
}

bool CHyprBar::inputIsValid() {
    if (!barActive())
        return false;

    if (g_pSeatManager->m_seatGrab && !g_pSeatManager->m_seatGrab->accepts(m_pWindow->wlSurface()->resource()))
        return false;

    const auto MOUSE    = g_pInputManager->getMouseCoordsInternal();
    auto       PMONITOR = Desktop::focusState()->monitor();

    if (!PMONITOR)
        return false;

    Desktop::CViewHitTester hitTester{*Desktop::viewState()};

    const auto              WINDOWATCURSOR = hitTester.windowAt(MOUSE, Desktop::View::RESERVED_EXTENTS | Desktop::View::INPUT_EXTENTS | Desktop::View::ALLOW_FLOATING);

    auto                    focusState = Desktop::focusState();
    auto                    window     = focusState->window();

    if (WINDOWATCURSOR != m_pWindow && m_pWindow != window)
        return false;

    PHLLS    foundSurface = nullptr;
    Vector2D surfaceCoords;

    // Check Top Layer
    hitTester.layerSurfaceAt(MOUSE, &PMONITOR->m_layerSurfaceLayers[ZWLR_LAYER_SHELL_V1_LAYER_TOP], &surfaceCoords, &foundSurface);
    if (foundSurface)
        return false;

    // Check Overlay Layer
    hitTester.layerSurfaceAt(MOUSE, &PMONITOR->m_layerSurfaceLayers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY], &surfaceCoords, &foundSurface);
    if (foundSurface)
        return false;

    return true;
}

void CHyprBar::onMouseButton(Event::SCallbackInfo& info, IPointer::SButtonEvent e) {
    // a release must reach us wherever the pointer ended up (a drag can end over another layer, e.g. a panel at the
    // screen edge, which input validation would reject)
    if (e.state != WL_POINTER_BUTTON_STATE_PRESSED && e.button != BTN_RIGHT && (m_bDraggingThis || m_bCancelledDown || m_bDragPending)) {
        handleUpEvent(info);
        return;
    }

    if (!inputIsValid())
        return;

    if (e.state != WL_POINTER_BUTTON_STATE_PRESSED) {
        if (e.button != BTN_RIGHT)
            handleUpEvent(info);
        return;
    }

    if (e.button == BTN_RIGHT) {
        // right click on the bar: the window menu
        const auto HEIGHT = g_pGlobalState->config.barHeight->value();
        if (VECINRECT(cursorRelativeToBar(), 0, 0, assignedBoxGlobal().w, HEIGHT - 1)) {
            if (Desktop::focusState()->window() != m_pWindow.lock())
                Desktop::focusState()->fullWindowFocus(m_pWindow.lock(), Desktop::FOCUS_REASON_CLICK);
            info.cancelled = true;
            runAction("menu");
        }
        return;
    }

    handleDownEvent(info, std::nullopt);
}

void CHyprBar::onTouchDown(Event::SCallbackInfo& info, ITouch::SDownEvent e) {
    // Don't do anything if you're already grabbed a window with another finger
    if (!inputIsValid() || e.touchID != 0)
        return;

    handleDownEvent(info, e);
}

void CHyprBar::onTouchUp(Event::SCallbackInfo& info, ITouch::SUpEvent e) {
    if (!m_bDragPending || !m_bTouchEv || e.touchID != m_touchId)
        return;

    handleUpEvent(info);
}

// Wheel over the bar: up raises the window, down lowers it (KWin's titlebar wheel action).
void CHyprBar::onMouseAxis(Event::SCallbackInfo& info, const IPointer::SAxisEvent& e) {
    if (e.axis != WL_POINTER_AXIS_VERTICAL_SCROLL || e.delta == 0.0 || !validMapped(m_pWindow) || !inputIsValid())
        return;

    const auto HEIGHT = g_pGlobalState->config.barHeight->value();
    const auto COORDS = cursorRelativeToBar();
    if (!VECINRECT(COORDS, 0, 0, assignedBoxGlobal().w, HEIGHT - 1))
        return;

    info.cancelled = true;
    runAction(e.delta < 0 ? "raise" : "lower");
}

void CHyprBar::onMouseMove(Vector2D coords) {
    if (m_bDraggingThis && !m_bTouchEv)
        maybeUnsnap();

    // ensure proper redraws of button icons on hover when using hardware cursors
    damageOnButtonHover();

    if (!m_bDragPending || m_bTouchEv || !validMapped(m_pWindow) || m_touchId != 0)
        return;

    m_bDragPending = false;
    handleMovement();
}

void CHyprBar::onTouchMove(Event::SCallbackInfo& info, ITouch::SMotionEvent e) {
    if (!m_bDragPending || !m_bTouchEv || !validMapped(m_pWindow) || e.touchID != m_touchId)
        return;

    auto PMONITOR     = m_pWindow->m_monitor.lock();
    PMONITOR          = PMONITOR ? PMONITOR : Desktop::focusState()->monitor();
    const auto COORDS = Vector2D(PMONITOR->m_position.x + e.pos.x * PMONITOR->m_size.x, PMONITOR->m_position.y + e.pos.y * PMONITOR->m_size.y);

    if (!m_bDraggingThis) {
        // Initial setup for dragging a window.
        g_pKeybindManager->m_dispatchers["setfloating"]("activewindow");
        g_pKeybindManager->m_dispatchers["resizewindowpixel"]("exact 50% 50%,activewindow");
        // pin it so you can change workspaces while dragging a window
        g_pKeybindManager->m_dispatchers["pin"]("activewindow");
    }
    g_pKeybindManager->m_dispatchers["movewindowpixel"](std::format("exact {} {},activewindow", (int)(COORDS.x - (assignedBoxGlobal().w / 2)), (int)COORDS.y));
    m_bDraggingThis = true;
}

void CHyprBar::handleDownEvent(Event::SCallbackInfo& info, std::optional<ITouch::SDownEvent> touchEvent) {
    m_bTouchEv = touchEvent.has_value();
    if (m_bTouchEv)
        m_touchId = touchEvent.value().touchID;

    const auto PWINDOW = m_pWindow.lock();

    auto       COORDS = cursorRelativeToBar();
    if (m_bTouchEv) {
        ITouch::SDownEvent e        = touchEvent.value();
        PHLMONITOR         PMONITOR = nullptr;
        for (auto& m : State::monitorState()->monitors()) {
            if (m->m_name == (!e.device->m_boundOutput.empty() ? e.device->m_boundOutput : "")) {
                PMONITOR = m;
                break;
            }
        }
        PMONITOR = PMONITOR ? PMONITOR : Desktop::focusState()->monitor();
        COORDS   = Vector2D(PMONITOR->m_position.x + e.pos.x * PMONITOR->m_size.x, PMONITOR->m_position.y + e.pos.y * PMONITOR->m_size.y) - assignedBoxGlobal().pos();
    }

    const auto HEIGHT           = g_pGlobalState->config.barHeight->value();
    const auto BARBUTTONPADDING = g_pGlobalState->config.barButtonPadding->value();
    const auto BARPADDING       = g_pGlobalState->config.barPadding->value();
    const auto ALIGNBUTTONS     = g_pGlobalState->config.barButtonsAlignment->value();
    const auto ON_DOUBLE_CLICK  = g_pGlobalState->config.onDoubleClick->value();

    const bool BUTTONSRIGHT = ALIGNBUTTONS != "left";

    if (!VECINRECT(COORDS, 0, 0, assignedBoxGlobal().w, HEIGHT - 1)) {

        if (m_bDraggingThis) {
            if (m_bTouchEv)
                g_pKeybindManager->m_dispatchers["settiled"]("activewindow");
            g_pKeybindManager->m_dispatchers["mouse"]("0movewindow");
            Log::logger->log(Log::DEBUG, "[kwinish-bars] Dragging ended on {:x}", (uintptr_t)PWINDOW.get());
        }

        m_bDraggingThis = false;
        m_bDragPending  = false;
        m_bTouchEv      = false;
        return;
    }

    if (Desktop::focusState()->window() != PWINDOW)
        Desktop::focusState()->fullWindowFocus(PWINDOW, Desktop::FOCUS_REASON_CLICK);

    if (PWINDOW->m_isFloating)
        Desktop::windowState()->raise(PWINDOW);

    info.cancelled   = true;
    m_bCancelledDown = true;

    if (doButtonPress(BARPADDING, BARBUTTONPADDING, HEIGHT, COORDS, BUTTONSRIGHT))
        return;

    // click on the app icon: the window menu, under the icon
    if (const float ICON = iconSlot(); ICON > 0 && VECINRECT(COORDS, BARPADDING, (HEIGHT - ICON) / 2.0, BARPADDING + ICON, (HEIGHT + ICON) / 2.0)) {
        const auto BOX = assignedBoxGlobal();
        openMenu(BOX.x + BARPADDING, BOX.y + BOX.h);
        m_bDragPending = false;
        return;
    }

    if (!ON_DOUBLE_CLICK.empty() &&
        std::chrono::duration_cast<std::chrono::milliseconds>(Time::steadyNow() - m_lastMouseDown).count() < 400 /* Arbitrary delay I found suitable */) {
        runAction(ON_DOUBLE_CLICK);
        m_bDragPending = false;
    } else {
        m_lastMouseDown = Time::steadyNow();
        m_pressCursor   = g_pInputManager->getMouseCoordsInternal();
        // The compositor grabs the window under the pointer when the move starts. Waiting for the first motion (as
        // upstream does) lets a fast move leave the bar and grab whatever is below, so a floating window's drag
        // starts right at the press, while the pointer is still on its bar. Tiled windows keep the lazy start.
        if (PWINDOW->m_isFloating && !m_bTouchEv) {
            m_bDragPending = false;
            handleMovement();
        } else
            m_bDragPending = true;
    }
}

// Which snap zone the pointer is on: "left", "right", "top" (maximize), or nullptr. Same test for the preview and the snap.
const char* CHyprBar::edgeZone(const Vector2D& C, PHLMONITOR M) {
    const auto  EDGE = 3.0;
    const float MW   = M->m_size.x / M->m_scale;
    if (C.x <= M->m_position.x + EDGE)
        return "left";
    if (C.x >= M->m_position.x + MW - EDGE - 1)
        return "right";
    if (g_pGlobalState->config.snapTopEdge->value() && C.y <= M->m_position.y + EDGE)
        return "top";
    return nullptr;
}

// The layout zone (published by the core as "z:N") under the pointer, or "" ; the smallest wins where zones overlap.
static std::string layoutZoneAt(const Vector2D& C, const std::unordered_map<std::string, SZone>& zones) {
    std::string best;
    double      bestArea = 1e18;
    for (const auto& [name, z] : zones) {
        if (!name.starts_with("z:"))
            continue;
        if (C.x >= z.pos.x && C.x < z.pos.x + z.size.x && C.y >= z.pos.y && C.y < z.pos.y + z.size.y && z.size.x * z.size.y < bestArea) {
            best     = name;
            bestArea = z.size.x * z.size.y;
        }
    }
    return best;
}

// While a floating window is dragged by its bar, shade where releasing it would put it: the screen-edge zone under
// the pointer, or - with Shift held - every zone of the active layout, the one under the pointer filled.
void CHyprBar::drawSnapPreview(PHLMONITOR pMonitor) {
    const auto MIT = g_pGlobalState->zones.find(pMonitor->m_name);
    std::string signature;
    std::vector<std::pair<const SZone*, bool>> shown; // zone, hovered

    if (MIT != g_pGlobalState->zones.end() && m_bDraggingThis && !m_bTouchEv && g_pGlobalState->config.snapPreview->value() &&
        (m_pressCursor - g_pInputManager->getMouseCoordsInternal()).size() >= 10) {
        const auto M = Desktop::focusState()->monitor();
        const auto C = g_pInputManager->getMouseCoordsInternal();
        if (M && M == pMonitor) {
            if (g_pInputManager->getModsFromAllKBs() & HL_MODIFIER_SHIFT) {
                const auto HOVER = layoutZoneAt(C, MIT->second);
                for (const auto& [name, z] : MIT->second)
                    if (name.starts_with("z:"))
                        shown.emplace_back(&z, name == HOVER);
                signature = shown.empty() ? "" : "L:" + HOVER;
            } else if (const char* z = edgeZone(C, M)) {
                if (const auto ZIT = MIT->second.find(z); ZIT != MIT->second.end()) {
                    shown.emplace_back(&ZIT->second, true);
                    signature = z;
                }
            }
        }
    }

    if (signature != m_szPreviewZone) {
        // repaint everything the preview covered or covers: the whole monitor is simplest and only happens on a change
        g_pHyprRenderer->damageBox(CBox{pMonitor->m_position.x, pMonitor->m_position.y, pMonitor->m_size.x / pMonitor->m_scale, pMonitor->m_size.y / pMonitor->m_scale});
        m_szPreviewZone = signature;
    }

    const auto  W     = m_pWindow.lock();
    const float SCALE = pMonitor->m_scale;
    for (const auto& [zone, hovered] : shown) {
        CHyprColor fill = configColor(g_pGlobalState->config.previewColor->value());
        CHyprColor line = fill;
        line.a          = std::min(1.0, fill.a * 2.5);
        if (!hovered)
            fill.a *= 0.25; // zones you are not on: an outline and a hint of fill

        CPreviewPassElement::SPreviewData data;
        data.box    = CBox{zone->pos.x - pMonitor->m_position.x, zone->pos.y - pMonitor->m_position.y, zone->size.x, zone->size.y}.scale(SCALE).round();
        data.round  = W ? std::round(W->rounding() * SCALE) : 0;
        data.border = std::max(1, (int)std::round(2 * SCALE));
        data.fill   = fill;
        data.line   = line;
        g_pHyprRenderer->m_renderPass.add(makeUnique<CPreviewPassElement>(data));
    }
}

// Dragging a snapped window by its bar gives back the size it had before the snap (as KWin does): once the pointer has
// moved a few pixels, resize to the remembered size, keep the pointer at the same fraction of the bar, and restart the
// compositor's move so its grab offset matches the new geometry.
void CHyprBar::maybeUnsnap() {
    if (m_bUnsnapChecked)
        return;

    const auto W = m_pWindow.lock();
    if (!W)
        return;

    const auto C = g_pInputManager->getMouseCoordsInternal();
    if ((C - m_pressCursor).size() < 6)
        return;
    m_bUnsnapChecked = true;

    const auto IT = g_pGlobalState->restores.find((uintptr_t)W.get());
    if (IT == g_pGlobalState->restores.end())
        return;

    const auto R    = IT->second;
    const auto POS  = W->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    const auto SIZE = W->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    g_pGlobalState->restores.erase(IT);

    // only a window still sitting where the snap put it is "snapped"
    if (std::abs(POS.x - R.snapPos.x) > 4 || std::abs(POS.y - R.snapPos.y) > 4 || std::abs(SIZE.x - R.snapSize.x) > 4 || std::abs(SIZE.y - R.snapSize.y) > 4)
        return;

    const double RATIO = std::clamp((m_pressCursor.x - R.snapPos.x) / std::max(1.0, R.snapSize.x), 0.0, 1.0);
    const Vector2D NEWPOS = {C.x - RATIO * R.size.x, C.y - (m_pressCursor.y - R.snapPos.y)};

    (void)Config::Actions::resize(R.size, false, W);
    (void)Config::Actions::move(NEWPOS, false, W);
    m_dragStart = NEWPOS;

    g_pKeybindManager->changeMouseBindMode(MBIND_INVALID);
    g_pKeybindManager->changeMouseBindMode(MBIND_MOVE);
}

// Releasing a drag with the pointer on a screen edge snaps the window (left/right: half, top: maximized), like KWin's
// edge tiling. The window first goes back to where the drag began so "restore" returns there, then the Lua core
// (kwinish.lua, loaded by the user's config) does the placement. Without the core this is a no-op.
void CHyprBar::snapOnRelease() {
    const auto W = m_pWindow.lock();
    if (!W || !W->m_isFloating)
        return;

    const auto M = Desktop::focusState()->monitor();
    if (!M)
        return;

    const auto  C = g_pInputManager->getMouseCoordsInternal();
    std::string where;
    if (g_pInputManager->getModsFromAllKBs() & HL_MODIFIER_SHIFT) {
        // Shift held: the layout zone under the pointer ("z:3" is action "zone:3")
        if (const auto MIT = g_pGlobalState->zones.find(M->m_name); MIT != g_pGlobalState->zones.end())
            if (const auto Z = layoutZoneAt(C, MIT->second); !Z.empty())
                where = "zone:" + Z.substr(2);
    } else if (const char* zone = edgeZone(C, M))
        where = std::string_view(zone) == "left" ? "tile:left" : std::string_view(zone) == "right" ? "tile:right" : "tile:max";
    const auto ACTIVE = Desktop::focusState()->window();
    Log::logger->log(Log::DEBUG, "[kwinish-bars] snap check: window={:x} active={:x} cursor=({:.0},{:.0}) monitor={} where={}", (uintptr_t)W.get(), (uintptr_t)ACTIVE.get(), C.x, C.y,
                     M->m_name, where.empty() ? "none" : where);
    // a plain click on the bar must not snap
    if (where.empty() || (C - m_pressCursor).size() < 10)
        return;

    (void)Config::Actions::move(m_dragStart, false, W);
    Config::Supplementary::executor()->spawn(
        std::format("hyprctl eval \"require('kwinish').window_action('0x{:x}','{}')\"", (uintptr_t)W.get(), where));
}

void CHyprBar::handleUpEvent(Event::SCallbackInfo& info) {
    if (m_pWindow.lock() != Desktop::focusState()->window())
        return;

    if (m_bCancelledDown)
        info.cancelled = true;

    m_bCancelledDown = false;

    if (m_bDraggingThis) {
        g_pKeybindManager->changeMouseBindMode(MBIND_INVALID);
        m_bDraggingThis = false;
        if (m_bTouchEv)
            (void)Config::Actions::floatWindow(Config::Actions::eTogglableAction::TOGGLE_ACTION_DISABLE);
        else
            snapOnRelease();

        Log::logger->log(Log::DEBUG, "[kwinish-bars] Dragging ended on {:x}", (uintptr_t)m_pWindow.lock().get());
    }

    m_bDragPending = false;
    m_bTouchEv     = false;
    m_touchId      = 0;
}

void CHyprBar::handleMovement() {
    m_bUnsnapChecked = false;
    if (const auto W = m_pWindow.lock()) {
        m_dragStart = W->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
        // The compositor drags the focused window. With follow_mouse, a fast move off the bar onto another window
        // can hand focus to that window before this runs, and the drag would grab it instead (a tiled window then
        // floats and follows the pointer). Pin the focus on this bar's window.
        if (Desktop::focusState()->window() != W)
            Desktop::focusState()->fullWindowFocus(W, Desktop::FOCUS_REASON_CLICK);
    }
    g_pKeybindManager->changeMouseBindMode(MBIND_MOVE);
    m_bDraggingThis = true;
    Log::logger->log(Log::DEBUG, "[kwinish-bars] Dragging initiated on {:x}", (uintptr_t)m_pWindow.lock().get());
    return;
}

// Built-in actions run inside the compositor, so they cannot go stale when Hyprland's dispatcher syntax changes
// (upstream hyprbars shells out, which broke the Ryoku buttons on the Lua-era Hyprland).
// action: close maximize fullscreen minimize float pin raise lower menu, or exec:<shell command>.
static void dispatchIfKnown(const std::string& name, const std::string& arg) {
    if (g_pKeybindManager->m_dispatchers.contains(name))
        g_pKeybindManager->m_dispatchers[name](arg);
}

void CHyprBar::runAction(const std::string& action) {
    const auto W = m_pWindow.lock();
    if (!W)
        return;

    if (action == "close")
        (void)Config::Actions::closeWindow(W);
    else if (action == "float")
        (void)Config::Actions::floatWindow(Config::Actions::TOGGLE_ACTION_TOGGLE, W);
    else if (action == "pin")
        (void)Config::Actions::pinWindow(Config::Actions::TOGGLE_ACTION_TOGGLE, W);
    else if (action == "raise")
        (void)Config::Actions::alterZOrder("top", W);
    else if (action == "lower")
        (void)Config::Actions::alterZOrder("bottom", W);
    else if (action == "maximize")
        dispatchIfKnown("fullscreen", "1"); // the window was just focused by the click
    else if (action == "fullscreen")
        dispatchIfKnown("fullscreen", "0");
    else if (action == "minimize")
        dispatchIfKnown("movetoworkspacesilent", "special:minimized");
    else if (action == "menu")
        openMenu(g_pInputManager->getMouseCoordsInternal().x, assignedBoxGlobal().y + assignedBoxGlobal().h);
    else if (action.starts_with("exec:"))
        dispatchIfKnown("exec", action.substr(5));
}

// One layout for hit tests, hover, drawing and the title: left items (app icon, then left buttons) from the left edge,
// right buttons from the right edge, the title in between. All unscaled bar-local pixels.
float CHyprBar::iconSlot() {
    if (!g_pGlobalState->config.showIcon->value())
        return 0;
    const auto W = m_pWindow.lock();
    if (!W || W->m_class.empty())
        return 0;
    // an icon lookup that already failed for this class must not leave a gap before the title
    if (m_szIconClass == W->m_class && !m_pIconTex)
        return 0;
    return std::max(12, (int)g_pGlobalState->config.barHeight->value() - 12);
}

std::vector<CHyprBar::SBtnSlot> CHyprBar::layoutButtons(float barW, float& leftEnd, float& rightStart) {
    const float PAD         = g_pGlobalState->config.barPadding->value();
    const float BTNPAD      = g_pGlobalState->config.barButtonPadding->value();
    const bool  DEFAULTLEFT = g_pGlobalState->config.barButtonsAlignment->value() == "left";
    const float ICON        = iconSlot();

    float       left     = PAD + (ICON > 0 ? ICON + 6 : 0);
    float       rightOff = PAD;

    std::vector<SBtnSlot> slots;
    for (size_t i = 0; i < g_pGlobalState->buttons.size(); ++i) {
        const auto& b      = g_pGlobalState->buttons[i];
        const bool  onLeft = b.alignSet ? b.left : DEFAULTLEFT;
        if (onLeft) {
            slots.push_back({i, left, 0, true});
            left += BTNPAD + b.size;
        } else {
            slots.push_back({i, barW - BTNPAD - b.size - rightOff, BTNPAD, false});
            rightOff += BTNPAD + b.size;
        }
    }
    leftEnd    = left;
    rightStart = barW - rightOff;
    return slots;
}

bool CHyprBar::barActive() {
    if (!g_pGlobalState->config.enabled->value() || m_hidden)
        return false;
    const auto W = m_pWindow.lock();
    if (W && g_pGlobalState->hiddenBars.contains((uintptr_t)W.get()))
        return false;
    return W && (!g_pGlobalState->config.floatingOnly->value() || W->m_isFloating);
}

void CHyprBar::onFocusChanged() {
    m_bTitleColorChanged = true;
    damageEntire();
}

void CHyprBar::onFloatingChanged() {
    g_pDecorationPositioner->repositionDeco(this);
    damageEntire();
}

// (Re)builds the app icon texture when the window class changes.
void CHyprBar::updateIcon(float scale) {
    const auto W    = m_pWindow.lock();
    const float SZ  = iconSlot();
    const int   PX  = std::round(SZ * scale);
    if (!W || SZ <= 0) {
        m_pIconTex = nullptr;
        return;
    }
    if (W->m_class == m_szIconClass && PX == m_iIconPx)
        return;

    m_szIconClass = W->m_class;
    m_iIconPx     = PX;
    m_pIconTex    = nullptr;

    const auto path = KwIcon::findIconPath(W->m_class, PX);
    if (auto* surface = KwIcon::load(path, PX)) {
        m_pIconTex = g_pHyprRenderer->createTexture(surface);
        cairo_surface_destroy(surface);
    }
}

// The window menu is born from the bar, as in KWin: its top-left corner goes to (x, y), global layout coordinates
// (the bar's bottom edge, under the icon or under the pointer).
void CHyprBar::openMenu(double x, double y) {
    const auto W = m_pWindow.lock();
    if (!W)
        return;
    dispatchIfKnown("exec", std::format("kwinish-ui menu 0x{:x} --at {},{}", (uintptr_t)W.get(), (int)x, (int)y));
}

bool CHyprBar::doButtonPress(Config::INTEGER barPadding, Config::INTEGER barButtonPadding, Config::INTEGER barHeight, Vector2D COORDS, const bool BUTTONSRIGHT) {
    float leftEnd = 0, rightStart = 0;
    for (const auto& s : layoutButtons(assignedBoxGlobal().w, leftEnd, rightStart)) {
        const auto& b = g_pGlobalState->buttons[s.idx];
        const float y = std::floor((barHeight - b.size) / 2.0);
        if (VECINRECT(COORDS, std::floor(s.x), y, std::floor(s.x) + b.size + barButtonPadding, y + b.size)) {
            runAction(b.cmd);
            return true;
        }
    }
    return false;
}

void CHyprBar::renderBarTitle(const Vector2D& bufferSize, const float scale) {
    const auto COLORVAL = g_pGlobalState->config.textColor->value();
    const auto SIZE     = g_pGlobalState->config.barTextSize->value();
    const auto WEIGHT   = g_pGlobalState->config.barTextWeight->value();
    const auto FONT     = g_pGlobalState->config.barTextFont->value();
    const auto ALIGN    = g_pGlobalState->config.barTextAlign->value();
    const auto PAD      = g_pGlobalState->config.barPadding->value();

    const float barW = bufferSize.x / scale;
    float       leftEnd = 0, rightStart = 0;
    layoutButtons(barW, leftEnd, rightStart);

    // the title lives between the left items and the right buttons; centred titles must not touch either side
    const float room     = ALIGN == "left" ? rightStart - leftEnd - PAD : 2 * std::min(barW / 2 - leftEnd, rightStart - barW / 2) - PAD;
    const int   maxWidth = std::clamp(static_cast<int>(room * scale), 0, INT_MAX);
    const int   scaledSize = std::round(SIZE * scale);

    if (m_szLastTitle.empty() || maxWidth < 1) {
        m_pTextTex = nullptr;
        return;
    }

    const bool       FOCUSED = m_pWindow.lock() && m_pWindow.lock() == Desktop::focusState()->window();
    const CHyprColor COLOR   = m_bForcedTitleColor.value_or(configColor(FOCUSED ? COLORVAL : g_pGlobalState->config.inactiveTextColor->value()));
    m_pTextTex             = g_pHyprRenderer->renderText(m_szLastTitle, COLOR, scaledSize, false, FONT, maxWidth, WEIGHT.m_value);
}

size_t CHyprBar::getVisibleButtonCount(Config::INTEGER barButtonPadding, Config::INTEGER barPadding, const Vector2D& bufferSize, const float scale) {
    return g_pGlobalState->buttons.size(); // kept for the header; the layout no longer drops buttons
}

void CHyprBar::renderBarButtons(CBox* barBox, const float scale, const float a) {
    const auto INACTIVECOLOR = g_pGlobalState->config.inactiveButtonColor->value();
    const auto HEIGHT        = g_pGlobalState->config.barHeight->value();
    const auto PINNED        = m_pWindow.lock() && m_pWindow.lock()->m_pinned;
    const auto COORDS        = cursorRelativeToBar();
    const bool INVALIDATEICONS = m_bButtonsDirty || m_bWindowSizeChanged;

    float      leftEnd = 0, rightStart = 0;
    for (const auto& s : layoutButtons(barBox->w / scale, leftEnd, rightStart)) {
        auto&      button           = g_pGlobalState->buttons[s.idx];
        const auto scaledButtonSize = button.size * scale;

        auto       color = button.bgcol;

        if (INACTIVECOLOR > 0) {
            color = m_bWindowHasFocus ? color : configColor(INACTIVECOLOR);
            if (INVALIDATEICONS && button.userfg && button.iconTex)
                button.iconTex = nullptr;
        }

        const float y       = std::floor((HEIGHT - button.size) / 2.0);
        const bool  hovered = VECINRECT(COORDS, std::floor(s.x), y, std::floor(s.x) + button.size + g_pGlobalState->config.barButtonPadding->value(), y + button.size);
        if (button.cmd == "pin" && PINNED)
            color = configColor(g_pGlobalState->config.pinnedColor->value());
        else if (hovered)
            color = button.hasHover ? button.hoverCol : configColor(g_pGlobalState->config.buttonHoverColor->value());

        color.a *= a;

        CBox buttonBox = {barBox->x + (s.x + s.drawDx) * scale, barBox->y + (barBox->h - scaledButtonSize) / 2.0, scaledButtonSize, scaledButtonSize};
        buttonBox.round();

        if (color.a > 0)
            g_pHyprOpenGL->renderRect(buttonBox, color, {.round = static_cast<int>(std::round(scaledButtonSize / 2.0)), .roundingPower = 2.F});
    }
}

void CHyprBar::renderBarButtonsText(CBox* barBox, const float scale, const float a) {
    const auto ICONONHOVER = g_pGlobalState->config.iconOnHover->value();
    const auto HEIGHT      = g_pGlobalState->config.barHeight->value();
    const auto COORDS      = cursorRelativeToBar();

    float      leftEnd = 0, rightStart = 0;
    for (const auto& s : layoutButtons(barBox->w / scale, leftEnd, rightStart)) {
        auto&      button           = g_pGlobalState->buttons[s.idx];
        const auto scaledButtonSize = button.size * scale;

        const float y       = std::floor((HEIGHT - button.size) / 2.0);
        const bool  hovering = VECINRECT(COORDS, std::floor(s.x), y, std::floor(s.x) + button.size + g_pGlobalState->config.barButtonPadding->value(), y + button.size);

        if ((!button.iconTex || button.iconTex->m_texID == 0) && !button.icon.empty()) {
            auto fgcol = button.userfg ? button.fgcol : (button.bgcol.r + button.bgcol.g + button.bgcol.b < 1) ? CHyprColor(0xFFFFFFFF) : CHyprColor(0xFF000000);

            if (button.icon.starts_with("builtin:")) {
                // drawn glyph: same size and stroke for every button, independent of installed fonts
                if (auto* surface = KwIcon::glyph(button.icon.substr(8), std::round(scaledButtonSize), fgcol.r, fgcol.g, fgcol.b, fgcol.a)) {
                    button.iconTex = g_pHyprRenderer->createTexture(surface);
                    cairo_surface_destroy(surface);
                }
            } else
                button.iconTex = g_pHyprRenderer->renderText(button.icon, fgcol, std::round(button.size * 0.62 * scale), false, "sans", scaledButtonSize);
        }

        if (!button.iconTex || button.iconTex->m_texID == 0)
            continue;

        const auto iconX = barBox->x + (s.x + s.drawDx) * scale + scaledButtonSize / 2.0 - button.iconTex->m_size.x / 2.0;
        const auto iconY = barBox->y + barBox->height / 2.0 - button.iconTex->m_size.y / 2.0;
        CBox       pos   = {iconX, iconY, button.iconTex->m_size.x, button.iconTex->m_size.y};

        if (!ICONONHOVER || (ICONONHOVER && hovering))
            g_pHyprOpenGL->renderTexture(button.iconTex, pos, {.a = a});
    }
}

void CHyprBar::draw(PHLMONITOR pMonitor, const float& a) {
    const auto ENABLED = g_pGlobalState->config.enabled->value();

    if (m_bLastEnabledState != ENABLED) {
        m_bLastEnabledState = ENABLED;
        g_pDecorationPositioner->repositionDeco(this);
    }

    // floating_only changed without a config reload (e.g. set at runtime): re-place the bar
    const auto FLOATINGONLY = g_pGlobalState->config.floatingOnly->value();
    if (m_bLastFloatingOnly != FLOATINGONLY) {
        m_bLastFloatingOnly = FLOATINGONLY;
        g_pDecorationPositioner->repositionDeco(this);
    }

    if (!barActive() || !validMapped(m_pWindow) || !ENABLED)
        return;

    drawSnapPreview(pMonitor);

    const auto PWINDOW = m_pWindow.lock();

    if (!PWINDOW->m_ruleApplicator->decorate().valueOrDefault())
        return;

    auto data = CBarPassElement::SBarData{this, a};
    g_pHyprRenderer->m_renderPass.add(makeUnique<CBarPassElement>(data));
}

void CHyprBar::renderPass(PHLMONITOR pMonitor, const float& a) {
    const auto  PWINDOW = m_pWindow.lock();

    static auto PENABLEBLURGLOBAL = CConfigValue<Config::BOOL>("decoration:blur:enabled");
    const auto  BARCOLOR          = g_pGlobalState->config.barColor->value();
    const auto  HEIGHT            = g_pGlobalState->config.barHeight->value();
    const auto  PRECEDENCE        = g_pGlobalState->config.barPrecedenceOverBorder->value();
    const auto  ALIGNBUTTONS      = g_pGlobalState->config.barButtonsAlignment->value();
    const auto  ENABLETITLE       = g_pGlobalState->config.barTitleEnabled->value();
    const auto  ENABLEBLUR        = g_pGlobalState->config.barBlur->value();
    const auto  INACTIVECOLOR     = g_pGlobalState->config.inactiveButtonColor->value();

    if (INACTIVECOLOR > 0) {
        bool currentWindowFocus = PWINDOW == Desktop::focusState()->window();
        if (currentWindowFocus != m_bWindowHasFocus) {
            m_bWindowHasFocus = currentWindowFocus;
            m_bButtonsDirty   = true;
        }
    }

    const bool       FOCUSED    = PWINDOW == Desktop::focusState()->window();
    const CHyprColor DEST_COLOR = m_bForcedBarColor.value_or(configColor(FOCUSED ? BARCOLOR : g_pGlobalState->config.inactiveBarColor->value()));
    if (DEST_COLOR != m_cRealBarColor->goal())
        *m_cRealBarColor = DEST_COLOR;

    CHyprColor color = m_cRealBarColor->value();

    color.a *= a;
    const bool BUTTONSRIGHT = ALIGNBUTTONS != "left";
    const bool SHOULDBLUR   = ENABLEBLUR && *PENABLEBLURGLOBAL && color.a < 1.F;

    if (HEIGHT < 1) {
        m_iLastHeight = HEIGHT;
        return;
    }

    const auto PWORKSPACE      = PWINDOW->m_workspace;
    const auto WORKSPACEOFFSET = PWORKSPACE && !PWINDOW->m_pinned ? PWORKSPACE->m_renderOffset->value() : Vector2D();

    const auto ROUNDING = PWINDOW->rounding() + (PRECEDENCE ? 0 : PWINDOW->getRealBorderSize());

    const auto scaledRounding = ROUNDING > 0 ? ROUNDING * pMonitor->m_scale - 2 /* idk why but otherwise it looks bad due to the gaps */ : 0;

    m_seExtents = {{0, HEIGHT}, {}};

    const auto DECOBOX = assignedBoxGlobal();

    const auto BARBUF = DECOBOX.size() * pMonitor->m_scale;

    CBox       titleBarBox = {DECOBOX.x - pMonitor->m_position.x, DECOBOX.y - pMonitor->m_position.y, DECOBOX.w,
                              DECOBOX.h + ROUNDING * 3 /* to fill the bottom cuz we can't disable rounding there */};

    titleBarBox.translate(PWINDOW->m_floatingOffset).scale(pMonitor->m_scale).round();

    if (titleBarBox.w < 1 || titleBarBox.h < 1)
        return;

    g_pHyprOpenGL->scissor(titleBarBox);

    if (ROUNDING) {
        // the +1 is a shit garbage temp fix until renderRect supports an alpha matte
        CBox windowBox = {PWINDOW->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT).x + PWINDOW->m_floatingOffset.x - pMonitor->m_position.x + 1,
                          PWINDOW->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT).y + PWINDOW->m_floatingOffset.y - pMonitor->m_position.y + 1,
                          PWINDOW->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT).x - 2, PWINDOW->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT).y - 2};

        if (windowBox.w < 1 || windowBox.h < 1)
            return;

        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);

        g_pHyprOpenGL->setCapStatus(GL_STENCIL_TEST, true);

        glStencilFunc(GL_ALWAYS, 1, -1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        windowBox.translate(WORKSPACEOFFSET).scale(pMonitor->m_scale).round();
        g_pHyprOpenGL->renderRect(windowBox, CHyprColor(0, 0, 0, 0), {.round = scaledRounding, .roundingPower = m_pWindow->roundingPower()});
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glStencilFunc(GL_NOTEQUAL, 1, -1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }

    if (SHOULDBLUR)
        g_pHyprOpenGL->renderRect(titleBarBox, color, {.round = scaledRounding, .roundingPower = m_pWindow->roundingPower(), .blur = true, .blurA = a});
    else
        g_pHyprOpenGL->renderRect(titleBarBox, color, {.round = scaledRounding, .roundingPower = m_pWindow->roundingPower()});

    // render title
    if (ENABLETITLE && (m_szLastTitle != PWINDOW->m_title || m_bWindowSizeChanged || !m_pTextTex || m_pTextTex->m_texID == 0 || m_bTitleColorChanged)) {
        m_szLastTitle = PWINDOW->m_title;
        renderBarTitle(BARBUF, pMonitor->m_scale);
    }

    if (ROUNDING) {
        // cleanup stencil
        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);
        g_pHyprOpenGL->setCapStatus(GL_STENCIL_TEST, false);
        glStencilMask(-1);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
    }

    CBox textBox = {titleBarBox.x, titleBarBox.y, (int)BARBUF.x, (int)BARBUF.y};

    updateIcon(pMonitor->m_scale);
    if (m_pIconTex && m_pIconTex->m_texID != 0) {
        const auto PAD = g_pGlobalState->config.barPadding->value() * pMonitor->m_scale;
        CBox       iconBox = {textBox.x + PAD, textBox.y + (BARBUF.y - m_pIconTex->m_size.y) / 2.0, m_pIconTex->m_size.x, m_pIconTex->m_size.y};
        g_pHyprOpenGL->renderTexture(m_pIconTex, iconBox.round(), {.a = a});
    }

    if (ENABLETITLE && m_pTextTex) {
        const auto BARPADDING       = g_pGlobalState->config.barPadding->value();
        const auto BARBUTTONPADDING = g_pGlobalState->config.barButtonPadding->value();
        const auto ALIGN            = g_pGlobalState->config.barTextAlign->value();

        float leftEnd = 0, rightStart = 0;
        layoutButtons(BARBUF.x / pMonitor->m_scale, leftEnd, rightStart);

        const auto scaledBorderSize = PWINDOW->getRealBorderSize() * pMonitor->m_scale;
        const auto xOffset          = ALIGN == "left" ? std::round(leftEnd * pMonitor->m_scale) : std::round(((BARBUF.x - scaledBorderSize) / 2.0 - m_pTextTex->m_size.x / 2.0));
        const auto yOffset           = std::round((BARBUF.y - m_pTextTex->m_size.y) / 2.0);
        CBox       titleBox          = {textBox.x + xOffset, textBox.y + yOffset, m_pTextTex->m_size.x, m_pTextTex->m_size.y};

        g_pHyprOpenGL->renderTexture(m_pTextTex, titleBox, {.a = a});
    }

    renderBarButtons(&textBox, pMonitor->m_scale, a);
    m_bButtonsDirty = false;

    g_pHyprOpenGL->scissor(nullptr);

    renderBarButtonsText(&textBox, pMonitor->m_scale, a);

    m_bWindowSizeChanged = false;
    m_bTitleColorChanged = false;

    // dynamic updates change the extents
    if (m_iLastHeight != HEIGHT) {
        PWINDOW->layoutTarget()->recalc();
        m_iLastHeight = HEIGHT;
    }
}

eDecorationType CHyprBar::getDecorationType() {
    return DECORATION_CUSTOM;
}

void CHyprBar::updateWindow(PHLWINDOW pWindow) {
    damageEntire();
}

void CHyprBar::onConfigReloaded() {
    m_bButtonsDirty      = true;
    m_bTitleColorChanged = true;
    m_pTextTex           = nullptr;

    g_pDecorationPositioner->repositionDeco(this);
    damageEntire();
}

void CHyprBar::damageEntire() {
    g_pHyprRenderer->damageBox(assignedBoxGlobal());
}

Vector2D CHyprBar::cursorRelativeToBar() {
    return g_pInputManager->getMouseCoordsInternal() - assignedBoxGlobal().pos();
}

eDecorationLayer CHyprBar::getDecorationLayer() {
    return DECORATION_LAYER_UNDER;
}

uint64_t CHyprBar::getDecorationFlags() {
    return DECORATION_ALLOWS_MOUSE_INPUT | (g_pGlobalState->config.barPartOfWindow->value() ? DECORATION_PART_OF_MAIN_WINDOW : 0);
}

CBox CHyprBar::assignedBoxGlobal() {
    if (!validMapped(m_pWindow))
        return {};

    CBox box = m_bAssignedBox;
    box.translate(g_pDecorationPositioner->getEdgeDefinedPoint(DECORATION_EDGE_TOP, m_pWindow.lock()));

    const auto PWORKSPACE      = m_pWindow->m_workspace;
    const auto WORKSPACEOFFSET = PWORKSPACE && !m_pWindow->m_pinned ? PWORKSPACE->m_renderOffset->value() : Vector2D();

    return box.translate(WORKSPACEOFFSET);
}

PHLWINDOW CHyprBar::getOwner() {
    return m_pWindow.lock();
}

void CHyprBar::updateRules() {
    const auto PWINDOW              = m_pWindow.lock();
    auto       prevHidden           = m_hidden;
    auto       prevForcedTitleColor = m_bForcedTitleColor;

    m_bForcedBarColor   = std::nullopt;
    m_bForcedTitleColor = std::nullopt;
    m_hidden            = false;

    if (PWINDOW->m_ruleApplicator->m_otherProps.props.contains(g_pGlobalState->nobarRuleIdx))
        m_hidden = truthy(PWINDOW->m_ruleApplicator->m_otherProps.props.at(g_pGlobalState->nobarRuleIdx)->effect);
    if (PWINDOW->m_ruleApplicator->m_otherProps.props.contains(g_pGlobalState->barColorRuleIdx))
        m_bForcedBarColor = CHyprColor(Config::ParserUtils::parseColor(PWINDOW->m_ruleApplicator->m_otherProps.props.at(g_pGlobalState->barColorRuleIdx)->effect).value_or(0));
    if (PWINDOW->m_ruleApplicator->m_otherProps.props.contains(g_pGlobalState->titleColorRuleIdx))
        m_bForcedTitleColor = CHyprColor(Config::ParserUtils::parseColor(PWINDOW->m_ruleApplicator->m_otherProps.props.at(g_pGlobalState->titleColorRuleIdx)->effect).value_or(0));

    if (prevHidden != m_hidden)
        g_pDecorationPositioner->repositionDeco(this);
    if (prevForcedTitleColor != m_bForcedTitleColor)
        m_bTitleColorChanged = true;
}

void CHyprBar::damageOnButtonHover() {
    const auto HEIGHT = g_pGlobalState->config.barHeight->value();
    const auto COORDS = cursorRelativeToBar();

    bool       any = false;
    float      leftEnd = 0, rightStart = 0;
    for (const auto& s : layoutButtons(assignedBoxGlobal().w, leftEnd, rightStart)) {
        const auto& b = g_pGlobalState->buttons[s.idx];
        const float y = std::floor((HEIGHT - b.size) / 2.0);
        if (VECINRECT(COORDS, std::floor(s.x), y, std::floor(s.x) + b.size + g_pGlobalState->config.barButtonPadding->value(), y + b.size))
            any = true;
    }

    if (any != m_bButtonHovered) {
        m_bButtonHovered = any;
        damageEntire();
    }
}
