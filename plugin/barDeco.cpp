// SPDX-License-Identifier: BSD-3-Clause AND MIT
// Derived from hyprbars (hyprwm/hyprland-plugins, BSD-3-Clause, Copyright (c) 2023 Hypr Development); changes MIT. See NOTICE.
#include "barDeco.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/desktop/state/LayerState.hpp>
#include <hyprland/src/desktop/state/ViewHitTester.hpp>
#include <hyprland/src/desktop/view/window/Window.hpp>
#include <hyprland/src/desktop/view/window/WindowPresentation.hpp>
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
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>
#include <hyprland/src/layout/LayoutManager.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/state/MonitorState.hpp>
#include <hyprland/src/pointer/PointerManager.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopManager.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopTimer.hpp>
#include <hyprland/src/render/pass/RectPassElement.hpp>
#include <hyprland/src/render/pass/TexPassElement.hpp>

#include "globals.hpp"
#include "hostActions.hpp"
#include "barInput.hpp"
#include "BarPassElement.hpp"
#include "icon.hpp"
#include "PreviewPassElement.hpp"
#include "tooltip.hpp"

#include <climits>

using namespace Render::GL;

static CHyprColor configColor(Config::INTEGER color) {
    return CHyprColor{static_cast<uint64_t>(color)};
}

CHyprBar::CHyprBar(PHLWINDOW pWindow) : IHyprWindowDecoration(pWindow) {
    m_pWindow = pWindow;

    const auto PMONITOR         = pWindow->m_monitor.lock();
    PMONITOR->m_scheduledRecalc = true;

    // button and pointer events; touch is not handled by the bar in alpha-0 (it reaches the window as before the plugin)
    m_pMouseButtonCallback = Event::bus()->m_events.input.mouse.button.listen([&](IPointer::SButtonEvent e, Event::SCallbackInfo& info) { onMouseButton(info, e); });
    m_pMouseAxisCallback = Event::bus()->m_events.input.mouse.axis.listen([&](IPointer::SAxisEvent e, Event::SCallbackInfo& info) { onMouseAxis(info, e); });
    m_pMouseMoveCallback = Event::bus()->m_events.input.mouse.move.listen([&](Vector2D, Event::SCallbackInfo&) { damageOnButtonHover(); });

    Animation::mgr()->createAnimation(configColor(g_pGlobalState->config.barColor->value()), m_cRealBarColor, Config::animationTree()->getAnimationPropertyConfig("border"),
                                      pWindow, AVARDAMAGE_NONE);
    m_cRealBarColor->setUpdateCallback([&](auto) { damageEntire(); });
}

CHyprBar::~CHyprBar() {
    if (g_pGlobalState->tip.bar == m_self)
        tipHide(true);
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

// underPointer: this bar's window must be the one under the pointer (by default the focused window's bar also counts)
bool CHyprBar::inputIsValid(bool underPointer) {
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

    if (WINDOWATCURSOR != m_pWindow && (underPointer || m_pWindow != window))
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

// Presses on the bar: barInput.hpp decides and records the pair (main.cpp takes the release); this feeds it the pointer
// test and runs the result. Alpha-0 (openspec disable-titlebar-drag-alpha0): a press on the title never moves the window.
void CHyprBar::onMouseButton(Event::SCallbackInfo& info, IPointer::SButtonEvent e) {
    KwIn::SPress p{.button = e.button, .pressed = e.state == WL_POINTER_BUTTON_STATE_PRESSED, .taken = info.cancelled};
    if (!p.pressed || p.taken || (e.button != BTN_LEFT && e.button != BTN_RIGHT))
        return;
    std::optional<size_t> button;
    if (inputIsValid()) {
        p.valid           = true;
        const auto COORDS = cursorRelativeToBar();
        const auto IB     = iconBox();
        if (!VECINRECT(COORDS, 0, 0, assignedBoxGlobal().w, g_pGlobalState->config.barHeight->value() - 1))
            p.hit = KwIn::eHit::OUTSIDE;
        else if ((button = buttonAt(COORDS)))
            p.hit = KwIn::eHit::BUTTON;
        else if (!IB.empty() && VECINRECT(COORDS, IB.x, IB.y, IB.x + IB.w, IB.y + IB.h))
            p.hit = KwIn::eHit::ICON;
        else
            p.hit = KwIn::eHit::TITLE;
    }

    const auto ON_DOUBLE_CLICK = g_pGlobalState->config.onDoubleClick->value();
    struct SHost {
        CHyprBar&             bar;
        Event::SCallbackInfo& info;
        PHLWINDOW             w;
        std::optional<size_t> button;
        std::string           doubleClick;
        void                  cancel() {
            info.cancelled = true;
        }
        void focus() {
            if (w && Desktop::focusState()->window() != w)
                Desktop::focusState()->fullWindowFocus(w, Desktop::FOCUS_REASON_CLICK);
        }
        void raiseIfFloating() {
            if (w && w->isFloating())
                Desktop::windowState()->raise(w);
        }
        void run(KwIn::eAction a) {
            switch (a) {
                case KwIn::eAction::BUTTON: bar.runAction(g_pGlobalState->buttons[*button].cmd); break;
                case KwIn::eAction::ICON_MENU: {
                    // the window menu, under the icon
                    const auto BOX = bar.assignedBoxGlobal();
                    bar.openMenu(BOX.x + g_pGlobalState->config.barPadding->value(), BOX.y + BOX.h);
                    break;
                }
                case KwIn::eAction::BAR_MENU: bar.runAction("menu"); break; // right click on the bar
                case KwIn::eAction::DOUBLE_CLICK: bar.runAction(doubleClick); break;
                default: break;
            }
        }
    } host{*this, info, m_pWindow.lock(), button, ON_DOUBLE_CLICK};
    KwIn::apply(m_clicks.onPress(p, g_pGlobalState->pairs, !ON_DOUBLE_CLICK.empty(), Time::steadyNow()), host);
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

// Which snap zone the pointer is on: "left", "right", "top" (maximize), or nullptr. Same test for the preview and the snap.
const char* CHyprBar::edgeZone(const Vector2D& C, PHLMONITOR M) {
    // An edge shared with another monitor is a seam: the pointer crosses it, so a 3px strip is unreachable. Give it a wide strip.
    const auto  SEAM = 16.0, EDGE = 3.0;
    const float MW = M->m_size.x, MH = M->m_size.y; // already logical (pixels / scale)
    bool        seamLeft = false, seamRight = false;
    for (const auto& N : State::monitorState()->monitors()) {
        if (N == M || N->m_position.y >= M->m_position.y + MH || N->m_position.y + N->m_size.y <= M->m_position.y)
            continue;
        seamLeft |= std::abs(N->m_position.x + N->m_size.x - M->m_position.x) < 2;
        seamRight |= std::abs(N->m_position.x - (M->m_position.x + MW)) < 2;
    }
    if (C.x <= M->m_position.x + (seamLeft ? SEAM : EDGE))
        return "left";
    if (C.x >= M->m_position.x + MW - (seamRight ? SEAM : EDGE) - 1)
        return "right";
    if (g_pGlobalState->config.snapTopEdge->value() && C.y <= M->m_position.y + EDGE)
        return "top";
    return nullptr;
}

// The monitor under the pointer. The focused monitor follows the dragged window's centre, which is on the other side of a
// seam while the pointer is still on this one.
static PHLMONITOR monitorAtPointer(const Vector2D& C) {
    for (const auto& N : State::monitorState()->monitors())
        if (C.x >= N->m_position.x && C.x < N->m_position.x + N->m_size.x && C.y >= N->m_position.y && C.y < N->m_position.y + N->m_size.y)
            return N;
    return Desktop::focusState()->monitor();
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
// Alpha-0 has no bar drag, so nothing calls this; it stays compiled because its render port belongs to group R, and
// alpha-1 brings back the drag condition (bf2049a) with the drag.
void CHyprBar::drawSnapPreview(PHLMONITOR pMonitor) {
    const auto MIT = g_pGlobalState->zones.find(pMonitor->m_name);
    std::string signature;
    std::vector<std::pair<const SZone*, bool>> shown; // zone, hovered

    if (MIT != g_pGlobalState->zones.end() && g_pGlobalState->config.snapPreview->value()) {
        const auto C = g_pInputManager->getMouseCoordsInternal();
        const auto M = monitorAtPointer(C);
        if (M && M == pMonitor) {
            if (g_pInputManager->getModsFromAllKBs() & Input::eKeyboardModifiers::HL_MODIFIER_SHIFT) {
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
        g_pHyprRenderer->damageBox(CBox{pMonitor->m_position.x, pMonitor->m_position.y, pMonitor->m_size.x, pMonitor->m_size.y});
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
        data.round  = W ? std::round(W->presentation().rounding() * SCALE) : 0;
        data.border = std::max(1, (int)std::round(2 * SCALE));
        data.fill   = fill;
        data.line   = line;
        g_pHyprRenderer->m_renderPass.add(makeUnique<CPreviewPassElement>(data));
    }
}

// Built-in actions run inside the compositor, so they cannot go stale when Hyprland's dispatcher syntax changes
// (upstream hyprbars shells out, which broke the Ryoku buttons on the Lua-era Hyprland).
// action: close maximize fullscreen minimize float pin raise lower menu, or exec:<shell command>; anything else is ignored.
// maximize, fullscreen and minimize act on the focused window (the click just focused the bar's), as the 0.56.2
// dispatchers did; a failure is logged (hostActions.hpp) and changes nothing else.
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
        KwAct::check(KwAct::SITE_MAXIMIZE, Config::Actions::fullscreenWindow(Fullscreen::FSMODE_MAXIMIZED, true)); // the window was just focused by the click
    else if (action == "fullscreen")
        KwAct::check(KwAct::SITE_FULLSCREEN, Config::Actions::fullscreenWindow(Fullscreen::FSMODE_FULLSCREEN, true));
    else if (action == "minimize")
        KwAct::check(KwAct::SITE_MINIMIZE, Config::Actions::moveToWorkspace(std::string{"special:minimized"}, true));
    else if (action == "menu")
        openMenu(g_pInputManager->getMouseCoordsInternal().x, assignedBoxGlobal().y + assignedBoxGlobal().h);
    else if (action.starts_with("exec:"))
        KwAct::spawn(KwAct::SITE_EXEC_BUTTON, action.substr(5));
}

// One layout for hit tests, hover, drawing and the title: left items (app icon, then left buttons) from the left edge,
// right buttons from the right edge, the title in between. All unscaled bar-local pixels.
float CHyprBar::iconSlot() {
    if (!g_pGlobalState->config.showIcon->value())
        return 0;
    const auto W = m_pWindow.lock();
    if (!W || W->metadata().appID().empty())
        return 0;
    // an icon lookup that already failed for this class must not leave a gap before the title
    if (m_szIconClass == W->metadata().appID() && !m_pIconTex)
        return 0;
    return std::max(12, (int)g_pGlobalState->config.barHeight->value() - 12);
}

// The app icon, bar-local and unscaled (empty without an icon): a click opens the window menu, resting on it shows its tip.
CBox CHyprBar::iconBox() {
    const float ICON = iconSlot();
    if (ICON <= 0)
        return {};
    return {(double)g_pGlobalState->config.barPadding->value(), (g_pGlobalState->config.barHeight->value() - ICON) / 2.0, ICON, ICON};
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
    return W && (!g_pGlobalState->config.floatingOnly->value() || W->isFloating());
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
    if (W->metadata().appID() == m_szIconClass && PX == m_iIconPx)
        return;

    m_szIconClass = W->metadata().appID();
    m_iIconPx     = PX;
    m_pIconTex    = nullptr;

    // this runs in the render pass: a filesystem error while searching the icon themes must not escape into the compositor
    try {
        const auto path = KwIcon::findIconPath(W->metadata().appID(), PX);
        if (auto* surface = KwIcon::load(path, PX)) {
            m_pIconTex = g_pHyprRenderer->createTexture(surface);
            cairo_surface_destroy(surface);
        }
    } catch (const std::exception& e) {
        Log::logger->log(Log::DEBUG, "kwinish-bars", "icon lookup failed for {}: {}", W->metadata().appID(), e.what());
    }
}

// The window menu is born from the bar, as in KWin: its top-left corner goes to (x, y), global layout coordinates
// (the bar's bottom edge, under the icon or under the pointer).
void CHyprBar::openMenu(double x, double y) {
    const auto W = m_pWindow.lock();
    if (!W)
        return;
    KwAct::spawn(KwAct::SITE_MENU, std::format("kwinish-ui menu 0x{:x} --at {},{}", (uintptr_t)W.get(), (int)x, (int)y));
}

// The bar button under COORDS (bar-local, unscaled), as an index in buttons.
std::optional<size_t> CHyprBar::buttonAt(const Vector2D& COORDS) {
    const auto HEIGHT = g_pGlobalState->config.barHeight->value();
    const auto BTNPAD = g_pGlobalState->config.barButtonPadding->value();
    float      leftEnd = 0, rightStart = 0;
    for (const auto& s : layoutButtons(assignedBoxGlobal().w, leftEnd, rightStart)) {
        const auto& b = g_pGlobalState->buttons[s.idx];
        const float y = std::floor((HEIGHT - b.size) / 2.0);
        if (VECINRECT(COORDS, std::floor(s.x), y, std::floor(s.x) + b.size + BTNPAD, y + b.size))
            return s.idx;
    }
    return std::nullopt;
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
    const auto PINNED        = m_pWindow.lock() && static_cast<bool>(m_pWindow.lock()->m_state & Desktop::View::WINDOW_STATE_PINNED);
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
    const auto WORKSPACEOFFSET = PWORKSPACE && !static_cast<bool>(PWINDOW->m_state & Desktop::View::WINDOW_STATE_PINNED) ? PWORKSPACE->m_renderOffset->value() : Vector2D();

    const auto ROUNDING = PWINDOW->presentation().rounding() + (PRECEDENCE ? 0 : PWINDOW->presentation().borderSize());

    const auto scaledRounding = ROUNDING > 0 ? ROUNDING * pMonitor->m_scale - 2 /* idk why but otherwise it looks bad due to the gaps */ : 0;

    m_seExtents = {{0, HEIGHT}, {}};

    const auto DECOBOX = assignedBoxGlobal();

    const auto BARBUF = DECOBOX.size() * pMonitor->m_scale;

    CBox       titleBarBox = {DECOBOX.x - pMonitor->m_position.x, DECOBOX.y - pMonitor->m_position.y, DECOBOX.w,
                              DECOBOX.h + ROUNDING * 3 /* to fill the bottom cuz we can't disable rounding there */};

    titleBarBox.translate(PWINDOW->presentation().floatingOffset()).scale(pMonitor->m_scale).round();

    if (titleBarBox.w < 1 || titleBarBox.h < 1)
        return;

    g_pHyprOpenGL->scissor(titleBarBox);

    if (ROUNDING) {
        // the +1 is a shit garbage temp fix until renderRect supports an alpha matte
        CBox windowBox = {PWINDOW->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT).x + PWINDOW->presentation().floatingOffset().x - pMonitor->m_position.x + 1,
                          PWINDOW->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT).y + PWINDOW->presentation().floatingOffset().y - pMonitor->m_position.y + 1,
                          PWINDOW->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT).x - 2, PWINDOW->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT).y - 2};

        if (windowBox.w < 1 || windowBox.h < 1) {
            g_pHyprOpenGL->scissor(nullptr);
            return;
        }

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
    if (ENABLETITLE && (m_szLastTitle != PWINDOW->metadata().title() || m_bWindowSizeChanged || !m_pTextTex || m_pTextTex->m_texID == 0 || m_bTitleColorChanged)) {
        m_szLastTitle = PWINDOW->metadata().title();
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

        const auto scaledBorderSize = PWINDOW->presentation().borderSize() * pMonitor->m_scale;
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

    // dynamic updates change the extents; the positioner caches them and hl.config fires no config reload, so ask it again
    if (m_iLastHeight != HEIGHT) {
        g_pDecorationPositioner->repositionDeco(this);
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
    const auto WORKSPACEOFFSET = PWORKSPACE && !static_cast<bool>(m_pWindow->m_state & Desktop::View::WINDOW_STATE_PINNED) ? PWORKSPACE->m_renderOffset->value() : Vector2D();

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

    size_t     hovered = SIZE_MAX;
    float      leftEnd = 0, rightStart = 0;
    for (const auto& s : layoutButtons(assignedBoxGlobal().w, leftEnd, rightStart)) {
        const auto& b = g_pGlobalState->buttons[s.idx];
        const float y = std::floor((HEIGHT - b.size) / 2.0);
        if (VECINRECT(COORDS, std::floor(s.x), y, std::floor(s.x) + b.size + g_pGlobalState->config.barButtonPadding->value(), y + b.size)) {
            hovered = s.idx;
            break;
        }
    }

    if ((hovered != SIZE_MAX) != m_bButtonHovered) {
        m_bButtonHovered = hovered != SIZE_MAX;
        damageEntire();
    }
    if (hovered == SIZE_MAX)
        if (const auto IB = iconBox(); !IB.empty() && VECINRECT(COORDS, IB.x, IB.y, IB.x + IB.w, IB.y + IB.h))
            hovered = TIP_ICON;
    updateTip(hovered);
}

static void damageMonitor(PHLMONITOR M) {
    if (M)
        g_pHyprRenderer->damageBox(CBox{M->m_position.x, M->m_position.y, M->m_size.x, M->m_size.y});
}

void tipHide(bool forget) {
    auto& tip = g_pGlobalState->tip;
    if (tip.shown)
        damageMonitor(tip.monitor.lock()); // the box was measured while drawing: the whole monitor is simplest, once per tip
    tip.shown = false;
    tip.tex   = nullptr;
    if (tip.timer)
        tip.timer->updateTimeout(std::nullopt);
    if (forget) {
        tip.bar.reset();
        tip.button  = SIZE_MAX;
        tip.pressed = false;
        tip.text.clear();
    }
}

// Who owns the tooltip after a pointer motion. Only the bar whose window is really under the pointer takes it: two stacked
// bars would otherwise keep taking it from each other and the delay would never run out.
void CHyprBar::updateTip(size_t hovered) {
    auto&       tip  = g_pGlobalState->tip;
    const bool  MINE = tip.bar == m_self;
    std::string text;
    if (hovered != SIZE_MAX && inputIsValid(true)) {
        static const bool PT = KwTip::portuguese(getenv("LC_ALL"), getenv("LC_MESSAGES"), getenv("LANG"));
        const auto        W      = m_pWindow.lock();
        const std::string ACTION = hovered == TIP_ICON ? "menu" : g_pGlobalState->buttons[hovered].cmd;
        const bool        CHECKED = W && (ACTION == "pin" ? static_cast<bool>(W->m_state & Desktop::View::WINDOW_STATE_PINNED) : Fullscreen::controller()->isFullscreen(W, Fullscreen::FSMODE_MAXIMIZED));
        text = KwTip::text(ACTION, CHECKED, PT);
    }

    if (text.empty()) {
        if (MINE)
            tipHide(true);
        return;
    }
    if (MINE && tip.button == hovered) {
        if (tip.text != text) { // e.g. maximized from the keyboard under a resting pointer: Maximize became Restore
            tip.text = text;
            tip.tex  = nullptr;
            if (tip.shown)
                damageMonitor(tip.monitor.lock());
        }
        return;
    }

    tipHide(true);
    tip.bar     = m_self;
    tip.button  = hovered;
    tip.text    = text;
    tip.monitor = monitorAtPointer(g_pInputManager->getMouseCoordsInternal());
    if (!tip.timer) {
        // the callback only reads the global state; PLUGIN_EXIT takes the timer out of the event loop
        tip.timer = makeShared<CEventLoopTimer>(
            std::nullopt,
            [](SP<CEventLoopTimer>, void*) {
                auto& t = g_pGlobalState->tip;
                if (t.bar.expired() || t.pressed)
                    return;
                t.shown = true;
                damageMonitor(t.monitor.lock());
            },
            nullptr);
        g_pEventLoopManager->addTimer(tip.timer);
    }
    tip.timer->updateTimeout(std::chrono::milliseconds(700)); // Qt's and so KDE's tooltip delay
}

// Queued by main.cpp after every window (RENDER_POST_WINDOWS): drawn with the bar, it would sit under the bar's own window.
void CHyprBar::drawTip(PHLMONITOR M) {
    auto&      tip = g_pGlobalState->tip;
    const auto W   = m_pWindow.lock();
    if (!tip.shown || tip.monitor != M || (tip.button >= g_pGlobalState->buttons.size() && tip.button != TIP_ICON) || !barActive() || !validMapped(m_pWindow) ||
        !W->m_ruleApplicator->decorate().valueOrDefault())
        return;

    const float SCALE = M->m_scale;
    if (!tip.tex || tip.texScale != SCALE) {
        tip.tex      = g_pHyprRenderer->renderText(tip.text, configColor(g_pGlobalState->config.textColor->value()), std::round(g_pGlobalState->config.barTextSize->value() * SCALE),
                                                   false, g_pGlobalState->config.barTextFont->value(), 0, g_pGlobalState->config.barTextWeight->value().m_value);
        tip.texScale = SCALE;
    }
    if (!tip.tex || tip.tex->m_texID == 0)
        return;

    const auto  BAR    = assignedBoxGlobal();
    const auto  HEIGHT = g_pGlobalState->config.barHeight->value();
    float       leftEnd = 0, rightStart = 0;
    KwTip::SRect button;
    if (tip.button == TIP_ICON) {
        const auto IB = iconBox();
        if (IB.empty())
            return;
        button = {BAR.x + IB.x, BAR.y + IB.y, IB.w, IB.h};
    } else
        for (const auto& s : layoutButtons(BAR.w, leftEnd, rightStart))
            if (s.idx == tip.button) {
                const float SZ = g_pGlobalState->buttons[s.idx].size;
                button         = {BAR.x + s.x + s.drawDx, BAR.y + std::floor((HEIGHT - SZ) / 2.0), SZ, SZ};
            }

    const double PADX = 8, PADY = 4;
    const auto   R    = KwTip::place(button, {BAR.x, BAR.y, BAR.w, BAR.h}, tip.tex->m_size.x / SCALE + 2 * PADX, tip.tex->m_size.y / SCALE + 2 * PADY,
                                     {M->m_position.x, M->m_position.y, M->m_size.x, M->m_size.y});
    const CBox   BOX  = CBox{R.x - M->m_position.x, R.y - M->m_position.y, R.w, R.h}.scale(SCALE).round();

    CHyprColor fill = configColor(g_pGlobalState->config.barColor->value());
    fill.a          = std::max(fill.a, 0.95); // readable over any window
    CHyprColor line = configColor(g_pGlobalState->config.textColor->value());
    line.a *= 0.25;
    const int  ROUND = std::round(4 * SCALE);

    CRectPassElement::SRectData outer;
    outer.box   = BOX;
    outer.color = line;
    outer.round = ROUND;
    g_pHyprRenderer->m_renderPass.add(makeUnique<CRectPassElement>(outer));
    CRectPassElement::SRectData inner;
    inner.box   = BOX.copy().expand(-std::max(1.0, std::round((double)SCALE)));
    inner.color = fill;
    inner.round = std::max(0, ROUND - 1);
    g_pHyprRenderer->m_renderPass.add(makeUnique<CRectPassElement>(inner));

    CTexPassElement::SRenderData text;
    text.tex = tip.tex;
    text.box = CBox{BOX.x + std::round(PADX * SCALE), BOX.y + std::round(PADY * SCALE), tip.tex->m_size.x, tip.tex->m_size.y};
    g_pHyprRenderer->m_renderPass.add(makeUnique<CTexPassElement>(std::move(text)));
}
