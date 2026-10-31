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
    virtual ~CHyprBar();

    virtual SDecorationPositioningInfo getPositioningInfo();

    virtual void                       onPositioningReply(const SDecorationPositioningReply& reply);

    virtual void                       draw(PHLMONITOR, float const& a);

    virtual eDecorationType            getDecorationType();

    virtual void                       updateWindow(PHLWINDOW);

    virtual void                       damageEntire();

    virtual eDecorationLayer           getDecorationLayer();

    virtual uint64_t                   getDecorationFlags();

    bool                               m_bButtonsDirty = true;

    virtual std::string                getDisplayName();

    PHLWINDOW                          getOwner();

    void                               updateRules();
    void                               onFloatingChanged();
    void                               onFocusChanged();
    void                               onConfigReloaded();
    void                               drawTip(PHLMONITOR);

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

    PHLANIMVAR<CHyprColor>     m_cRealBarColor;

    Vector2D                   cursorRelativeToBar();

    void                       renderPass(PHLMONITOR, float const& a);
    void                       renderBarTitle(const Vector2D& bufferSize, const float scale);
    void renderBarButtons(CBox* barBox, const float scale, const float a);
    void renderBarButtonsText(CBox* barBox, const float scale, const float a);
    void damageOnButtonHover();
    void updateTip(size_t hovered);

    bool inputIsValid(bool underPointer = false);
    void onMouseButton(Event::SCallbackInfo& info, IPointer::SButtonEvent e);
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
    void                  drawSnapPreview(PHLMONITOR monitor);
    void onMouseAxis(Event::SCallbackInfo& info, const IPointer::SAxisEvent& e);
    std::optional<size_t> buttonAt(const Vector2D& coords);

    CBox assignedBoxGlobal();

    CHyprSignalListener m_pMouseButtonCallback;
    CHyprSignalListener m_pMouseMoveCallback;
    CHyprSignalListener m_pMouseAxisCallback;

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
