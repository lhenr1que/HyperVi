// SPDX-License-Identifier: BSD-3-Clause AND MIT
// Derived from hyprbars (hyprwm/hyprland-plugins, BSD-3-Clause, Copyright (c) 2023 Hypr Development); changes MIT. See NOTICE.
#pragma once

#define WLR_USE_UNSTABLE

#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/gl/GLTexture.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/desktop/rule/windowRule/WindowRule.hpp>
#include <hyprland/src/helpers/AnimatedVariable.hpp>
#include <hyprland/src/helpers/time/Time.hpp>
#include <hyprland/src/helpers/signal/Signal.hpp>
#include "globals.hpp"
#include "barInput.hpp"

#define private public
#include <hyprland/src/managers/input/InputManager.hpp>
#undef private

namespace Event {
    struct SCallbackInfo;
}

// Takes the button tooltip off the screen; forget: also drop the bar and button that own it.
void tipHide(bool forget);

class CHyprBar : public IHyprWindowDecoration {
  public:
    CHyprBar(PHLWINDOW);
    virtual ~CHyprBar() override;

    virtual SDecorationPositioningInfo getPositioningInfo() override;

    virtual void                       onPositioningReply(const SDecorationPositioningReply& reply) override;

    virtual void                       draw(Render::CRenderContext& ctx, PHLMONITOR, float const& a, const Render::SWindowRenderPresentation& presentation) override;

    virtual eDecorationType            getDecorationType() override;

    virtual void                       updateWindow(PHLWINDOW) override;

    virtual void                       damageEntire() override;

    virtual eDecorationLayer           getDecorationLayer() override;

    virtual uint64_t                   getDecorationFlags() override;

    bool                               m_bButtonsDirty = true;

    virtual std::string                getDisplayName() override;

    PHLWINDOW                          getOwner();

    void                               updateRules();
    void                               onFloatingChanged();
    void                               onFocusChanged();
    void                               onConfigReloaded();
    void                               drawTip(Render::CRenderContext& ctx, PHLMONITOR);
    void                               onShift();
    bool                               holdAtSeam(IPointer::SMotionEvent& e);

    WP<CHyprBar>                       m_self;

  private:
    SBoxExtents                m_seExtents;

    PHLWINDOWREF               m_pWindow;

    CBox                       m_bAssignedBox;

    SP<Render::ITexture>       m_pTextTex;

    bool                       m_bWindowSizeChanged = false;
    bool                       m_hidden             = false;
    bool                       m_bTitleColorChanged = false;
    bool                       m_bButtonHovered     = false;
    bool                       m_bLastEnabledState  = false;
    bool                       m_bLastFloatingOnly  = false;
    bool                       m_bWindowHasFocus    = false;
    std::optional<CHyprColor>  m_bForcedBarColor;
    std::optional<CHyprColor>  m_bForcedTitleColor;

    KwIn::SClicks              m_clicks;

    // the bar's drag (alpha-1, alt5-rc): barInput.hpp decides, the compositor ends it
    KwIn::SDrag                m_drag;
    bool                       m_bDragPending = false; // tiled, maximized or snapped: the drag starts on the first real motion
    KwIn::SUnsnap              m_unsnap;               // a snapped window easing back before its drag starts
    Vector2D                   m_dragStart;            // window position when the drag started
    Vector2D                   m_pressCursor;
    double                     m_resistPush = 0;       // pixels pushed against the seam so far
    double                     m_resistSeam = 0;       // x of that seam

    PHLANIMVAR<CHyprColor>     m_cRealBarColor;

    Vector2D                   cursorRelativeToBar();

    void                       renderPass(Render::CRenderContext& ctx, PHLMONITOR, float const& a);
    void                       renderBarTitle(const Vector2D& bufferSize, const float scale);
    void renderBarButtons(Render::CRenderContext& ctx, CBox* barBox, const float scale, const float a);
    void renderBarButtonsText(Render::CRenderContext& ctx, CBox* barBox, const float scale, const float a);
    void damageOnButtonHover();
    void updateTip(size_t hovered);

    bool inputIsValid(bool underPointer = false);
    void onMouseButton(Event::SCallbackInfo& info, IPointer::SButtonEvent e);
    void onMouseMove();
    void onTitlePress();
    void handleMovement();
    bool snappedHere(const PHLWINDOW& w);
    bool startUnsnap(const PHLWINDOW& w, const Vector2D& cursor);
    void applyUnsnap(const PHLWINDOW& w, const KwIn::SUnsnapStep& st);
    void snapOnRelease();
    void onDragEnded();
    bool dragGoing();
    bool pointerOverWindow();
    struct SBtnSlot {
        size_t idx;
        float  x;      // hit-test left edge, bar-local, unscaled
        float  drawDx; // offset from x to the drawn circle
        bool   left;
    };
    std::vector<SBtnSlot> layoutButtons(float barW, float& leftEnd, float& rightStart);
    float                 iconSlot();
    CBox                  iconBox();
    bool                  barActive();
    void                  updateIcon(float scale);
    void                  runAction(const std::string& action);
    void                  openMenu(double x, double y);
    const char*           edgeZone(const Vector2D& cursor, PHLMONITOR monitor);
    void                  drawSnapPreview(Render::CRenderContext& ctx, PHLMONITOR monitor);
    void onMouseAxis(Event::SCallbackInfo& info, const IPointer::SAxisEvent& e);
    std::optional<size_t> buttonAt(const Vector2D& coords);

    CBox assignedBoxGlobal();

    CHyprSignalListener m_pMouseButtonCallback;
    CHyprSignalListener m_pMouseMoveCallback;
    CHyprSignalListener m_pMouseAxisCallback;
    CHyprSignalListener m_pDragEndedCallback;

    std::string         m_szLastTitle;
    SP<Render::ITexture> m_pIconTex;
    std::string         m_szIconClass;
    int                 m_iIconPx = 0;
    std::string         m_szPreviewZone;

    // store hover state for buttons as a bitfield
    unsigned int m_iButtonHoverState = 0;

    // for dynamic updates
    int    m_iLastHeight = 0;

    size_t getVisibleButtonCount(Config::INTEGER barButtonPadding, Config::INTEGER barPadding, const Vector2D& bufferSize, const float scale);

    friend class CBarPassElement;
};
