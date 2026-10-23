#pragma once

#include <unordered_map>

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/render/Texture.hpp>
#include <hyprland/src/config/values/types/BoolValue.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/config/values/types/StringValue.hpp>
#include <hyprland/src/config/values/types/ColorValue.hpp>
#include <hyprland/src/config/values/types/FontWeightValue.hpp>

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

struct SGlobalState {
    std::vector<SHyprButton>  buttons;
    std::vector<WP<CHyprBar>> bars;
    std::unordered_map<std::string, std::unordered_map<std::string, SZone>> zones; // monitor name -> zone name, filled by the Lua core
    std::unordered_map<uintptr_t, SRestore> restores; // by window pointer, filled by the Lua core
    uint32_t                  nobarRuleIdx      = 0;
    uint32_t                  barColorRuleIdx   = 0;
    uint32_t                  titleColorRuleIdx = 0;

    struct {
        SP<Config::Values::CColorValue>      barColor, textColor, inactiveButtonColor;
        SP<Config::Values::CIntValue>        barHeight;
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
