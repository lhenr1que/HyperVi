// SPDX-License-Identifier: BSD-3-Clause AND MIT
// Derived from hyprbars (hyprwm/hyprland-plugins, BSD-3-Clause, Copyright (c) 2023 Hypr Development); changes MIT. See NOTICE.
#pragma once

#include <unordered_map>
#include <unordered_set>

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/render/Texture.hpp>
#include <hyprland/src/config/values/types/BoolValue.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/config/values/types/StringValue.hpp>
#include <hyprland/src/config/values/types/ColorValue.hpp>
#include <hyprland/src/config/values/types/FontWeightValue.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopTimer.hpp>

inline HANDLE PHANDLE = nullptr;

struct SHyprButton {
    std::string          cmd     = "";
    bool                 userfg  = false;
    CHyprColor           fgcol   = CHyprColor(0, 0, 0, 0);
    CHyprColor           bgcol   = CHyprColor(0, 0, 0, 0);
    float                size    = 10;
    std::string          icon    = "";
    SP<Render::ITexture> iconTex;
    bool                 alignSet = false; // false: follow bar_buttons_alignment
    bool                 left     = false;
    bool                 hasHover = false; // false: button_hover_color
    CHyprColor           hoverCol = CHyprColor(0, 0, 0, 0);
};

struct SRestore {
    Vector2D pos, size;         // geometry before the window was snapped
    Vector2D snapPos, snapSize; // geometry the snap gave it
};

struct SZone {
    Vector2D pos, size; // global layout coordinates; includes the title bar
};

class CHyprBar;

inline constexpr size_t TIP_ICON = SIZE_MAX - 1; // tip.button for the app icon

struct SGlobalState {
    std::vector<SHyprButton>  buttons;
    std::vector<WP<CHyprBar>> bars;
    std::unordered_map<std::string, std::unordered_map<std::string, SZone>> zones; // monitor name -> zone name, filled by the Lua core
    // Move/Resize from the window menu: the compositor's mouse-bind drag follows the pointer until the next button press
    bool                      grabActive       = false;
    bool                      grabSwallowPress = false; // the press that ended a grab is hidden from the bars and the app, and so is its release
    bool                      warnedNoCore = false; // told the user once that the Lua core never published zones
    std::unordered_set<uintptr_t> hiddenBars; // windows whose bar the menu turned off ("no title bar and frame")
    std::unordered_map<uintptr_t, SRestore> restores; // by window pointer, filled by the Lua core
    uint32_t                  nobarRuleIdx      = 0;
    uint32_t                  barColorRuleIdx   = 0;
    uint32_t                  titleColorRuleIdx = 0;

    // The button tooltip: one pointer, one tip, owned by the bar whose button the pointer rests on, drawn after the windows
    struct {
        WP<CHyprBar>         bar;
        size_t               button  = SIZE_MAX; // index in buttons, or TIP_ICON
        std::string          text;
        PHLMONITORREF        monitor;
        bool                 shown   = false; // the delay has passed
        bool                 pressed = false; // a click hid it: it stays hidden until the pointer leaves the button
        SP<Render::ITexture> tex;
        float                texScale = 0;
        SP<CEventLoopTimer>  timer; // the event loop holds it too: removed in PLUGIN_EXIT
    } tip;

    struct {
        SP<Config::Values::CColorValue>      barColor, textColor, inactiveButtonColor;
        SP<Config::Values::CIntValue>        barHeight, seamResistance;
        SP<Config::Values::CIntValue>        barTextSize;
        SP<Config::Values::CFontWeightValue> barTextWeight;
        SP<Config::Values::CIntValue>        barPadding;
        SP<Config::Values::CIntValue>        barButtonPadding;
        SP<Config::Values::CBoolValue>       barBlur, barTitleEnabled, barPartOfWindow, barPrecedenceOverBorder, enabled, iconOnHover, floatingOnly, showIcon, defaultButtons, snapTopEdge, snapPreview;
        SP<Config::Values::CColorValue>      buttonHoverColor, pinnedColor, inactiveBarColor, inactiveTextColor, previewColor;
        SP<Config::Values::CStringValue>     barTextFont, barTextAlign, barButtonsAlignment, onDoubleClick;
    } config;
};

inline UP<SGlobalState> g_pGlobalState;
