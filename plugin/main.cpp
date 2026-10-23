#define WLR_USE_UNSTABLE

#include <unistd.h>

#include <any>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/config/shared/parserUtils/ParserUtils.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/desktop/rule/windowRule/WindowRuleEffectContainer.hpp>
#include <hyprland/src/config/lua/bindings/LuaBindingsInternal.hpp>
#include <hyprland/src/config/lua/types/LuaConfigColor.hpp>
#include <hyprland/src/state/MonitorState.hpp>

#include <hyprutils/string/VarList.hpp>

#include <algorithm>

#include "barDeco.hpp"
#include "globals.hpp"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

static void onNewWindow(PHLWINDOW window) {
    if (!window->m_X11DoesntWantBorders) {
        if (std::ranges::any_of(window->m_windowDecorations, [](const auto& d) { return d->getDisplayName() == "Hyprbar" || d->getDisplayName() == "KwinishBar"; }))
            return;

        auto bar = makeUnique<CHyprBar>(window);
        g_pGlobalState->bars.emplace_back(bar);
        bar->m_self = bar;
        HyprlandAPI::addWindowDecoration(PHANDLE, window, std::move(bar));
    }
}

static void onPreConfigReload() {
    g_pGlobalState->buttons.clear();
}

// Breeze-like defaults when the config adds no button: pin at the left, close/maximize/minimize at the right
// (the first button added sits outermost on the right).
static void installDefaultButtons() {
    if (!g_pGlobalState->buttons.empty() || !g_pGlobalState->config.defaultButtons->value())
        return;

    const auto none = CHyprColor(0, 0, 0, 0);
    const auto fg   = CHyprColor(0.94, 0.94, 0.96, 1.0);
    auto       add  = [&](std::string icon, std::string cmd, bool left, std::optional<CHyprColor> hover = std::nullopt) {
        SHyprButton b;
        b.icon = std::move(icon);
        b.cmd = std::move(cmd);
        b.bgcol = none;
        b.fgcol = fg;
        b.userfg = true;
        b.size = 16;
        b.alignSet = true;
        b.left = left;
        if (hover) {
            b.hasHover = true;
            b.hoverCol = *hover;
        }
        g_pGlobalState->buttons.push_back(std::move(b));
    };
    add("\u25C6", "pin", true);
    add("\u2715", "close", false, CHyprColor(0.90, 0.28, 0.30, 1.0));
    add("\u25A1", "maximize", false);
    add("\u2212", "minimize", false);
}

static void onConfigReloaded() {
    installDefaultButtons();

    for (auto& b : g_pGlobalState->bars) {
        if (!b)
            continue;

        b->onConfigReloaded();
    }
}

static void onUpdateWindowRules(PHLWINDOW window) {
    const auto BARIT = std::find_if(g_pGlobalState->bars.begin(), g_pGlobalState->bars.end(), [window](const auto& bar) { return bar->getOwner() == window; });

    if (BARIT == g_pGlobalState->bars.end())
        return;

    (*BARIT)->updateRules();
    window->updateWindowDecos();
}

Hyprlang::CParseResult onNewButton(const char* K, const char* V) {
    std::string                 v = V;
    Hyprutils::String::CVarList vars(v);

    Hyprlang::CParseResult      result;

    // hyprbars-button = bgcolor, size, icon, action, fgcolor

    if (vars[0].empty() || vars[1].empty()) {
        result.setError("bgcolor and size cannot be empty");
        return result;
    }

    float size = 10;
    try {
        size = std::stof(vars[1]);
    } catch (std::exception& e) {
        result.setError("failed to parse size");
        return result;
    }

    bool userfg  = false;
    auto fgcolor = Config::ParserUtils::parseColor("rgb(ffffff)");
    auto bgcolor = Config::ParserUtils::parseColor(vars[0]);

    if (!bgcolor) {
        result.setError("invalid bgcolor");
        return result;
    }

    if (vars.size() == 5) {
        userfg  = true;
        fgcolor = Config::ParserUtils::parseColor(vars[4]);
    }

    if (!fgcolor) {
        result.setError("invalid fgcolor");
        return result;
    }

    g_pGlobalState->buttons.push_back(SHyprButton{vars[3], userfg, *fgcolor, *bgcolor, size, vars[2]});

    for (auto& b : g_pGlobalState->bars) {
        b->m_bButtonsDirty = true;
    }

    return result;
}

int newLuaButton(lua_State* L) {
    if (!lua_istable(L, 1))
        return Config::Lua::Bindings::Internal::configError(L, "add_button: expected a table { bg_color, fg_color, size, icon, action }");

    SHyprButton button;

    {
        Hyprutils::Utils::CScopeGuard x([L] { lua_pop(L, 1); });

        lua_getfield(L, 1, "bg_color");

        Config::Lua::CLuaConfigColor parser(0);
        auto                         err = parser.parse(L);
        if (err.errorCode != Config::Lua::PARSE_ERROR_OK)
            return Config::Lua::Bindings::Internal::configError(L, "add_button: failed to parse bg_color");

        button.bgcol = parser.parsed();
    }

    {
        Hyprutils::Utils::CScopeGuard x([L] { lua_pop(L, 1); });

        lua_getfield(L, 1, "fg_color");

        Config::Lua::CLuaConfigColor parser(0);
        auto                         err = parser.parse(L);
        if (err.errorCode != Config::Lua::PARSE_ERROR_OK)
            return Config::Lua::Bindings::Internal::configError(L, "add_button: failed to parse fg_color");

        button.userfg = true;
        button.fgcol = parser.parsed();
    }

    {
        Hyprutils::Utils::CScopeGuard x([L] { lua_pop(L, 1); });

        lua_getfield(L, 1, "size");

        if (!lua_isnumber(L, -1))
            return Config::Lua::Bindings::Internal::configError(L, "add_button: size must be an integer");

        button.size = lua_tointeger(L, -1);
    }

    {
        Hyprutils::Utils::CScopeGuard x([L] { lua_pop(L, 1); });

        lua_getfield(L, 1, "icon");

        if (!lua_isstring(L, -1))
            return Config::Lua::Bindings::Internal::configError(L, "add_button: icon must be a string");

        button.icon = lua_tostring(L, -1);
    }

    {
        Hyprutils::Utils::CScopeGuard x([L] { lua_pop(L, 1); });

        lua_getfield(L, 1, "action");

        if (!lua_isstring(L, -1))
            return Config::Lua::Bindings::Internal::configError(L, "add_button: action must be a string");

        button.cmd = lua_tostring(L, -1);
    }

    {
        Hyprutils::Utils::CScopeGuard x([L] { lua_pop(L, 1); });

        lua_getfield(L, 1, "align");

        if (lua_isstring(L, -1)) {
            const std::string a = lua_tostring(L, -1);
            button.alignSet     = a == "left" || a == "right";
            button.left         = a == "left";
        }
    }

    {
        Hyprutils::Utils::CScopeGuard x([L] { lua_pop(L, 1); });

        lua_getfield(L, 1, "hover_color");

        if (!lua_isnil(L, -1)) {
            Config::Lua::CLuaConfigColor parser(0);
            auto                         err = parser.parse(L);
            if (err.errorCode != Config::Lua::PARSE_ERROR_OK)
                return Config::Lua::Bindings::Internal::configError(L, "add_button: failed to parse hover_color");

            button.hasHover = true;
            button.hoverCol = parser.parsed();
        }
    }

    // applying the same settings twice (live apply plus a reload) must not stack identical buttons
    if (std::ranges::any_of(g_pGlobalState->buttons, [&](const auto& b) { return b.icon == button.icon && b.cmd == button.cmd; }))
        return 0;

    g_pGlobalState->buttons.push_back(std::move(button));

    for (auto& b : g_pGlobalState->bars) {
        b->m_bButtonsDirty = true;
    }

    return 0;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH        = __hyprland_api_get_hash();
    const std::string CLIENT_HASH = __hyprland_api_get_client_hash();

    if (HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[kwinish-bars] Failure in initialization: Version mismatch (headers ver is not equal to running hyprland ver)",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[kwinish-bars] Version mismatch");
    }

    g_pGlobalState                    = makeUnique<SGlobalState>();
    g_pGlobalState->nobarRuleIdx      = Desktop::Rule::windowEffects()->registerEffect("kwinish:no_bar");
    g_pGlobalState->barColorRuleIdx   = Desktop::Rule::windowEffects()->registerEffect("kwinish:bar_color");
    g_pGlobalState->titleColorRuleIdx = Desktop::Rule::windowEffects()->registerEffect("kwinish:title_color");

    static auto P  = Event::bus()->m_events.window.open.listen([&](PHLWINDOW w) { onNewWindow(w); });
    static auto P3 = Event::bus()->m_events.window.updateRules.listen([&](PHLWINDOW w) { onUpdateWindowRules(w); });

    g_pGlobalState->config.barColor            = makeShared<Config::Values::CColorValue>("plugin:kwinish:bar_color", "Change the bar color", 0xe01c1c22);
    g_pGlobalState->config.textColor           = makeShared<Config::Values::CColorValue>("plugin:kwinish:col.text", "Change the text color", 0xfff0f0f6);
    g_pGlobalState->config.inactiveButtonColor = makeShared<Config::Values::CColorValue>(
        "plugin:kwinish:inactive_button_color", "Change the inactive button's color. 0x00000000 means unset", 0x00000000);
    g_pGlobalState->config.barHeight       = makeShared<Config::Values::CIntValue>("plugin:kwinish:bar_height", "Change the bar's height", 28);
    g_pGlobalState->config.barTextSize     = makeShared<Config::Values::CIntValue>("plugin:kwinish:bar_text_size", "Change the bar's text size", 12);
    g_pGlobalState->config.barTextWeight   = makeShared<Config::Values::CFontWeightValue>("plugin:kwinish:bar_text_weight", "Bar's title text weight (e.g. \"bold\" or an integer 100-1000)", 400);
    g_pGlobalState->config.barTitleEnabled = makeShared<Config::Values::CBoolValue>("plugin:kwinish:bar_title_enabled", "Whether to enable titles in the bar", true);
    g_pGlobalState->config.barBlur         = makeShared<Config::Values::CBoolValue>("plugin:kwinish:bar_blur", "Whether to enable blur of the bar", false);
    g_pGlobalState->config.barTextFont     = makeShared<Config::Values::CStringValue>("plugin:kwinish:bar_text_font", "Bar's text font", "Sans");
    g_pGlobalState->config.barTextAlign    = makeShared<Config::Values::CStringValue>("plugin:kwinish:bar_text_align", "Bar's text alignment", "left");
    g_pGlobalState->config.barPartOfWindow =
        makeShared<Config::Values::CBoolValue>("plugin:kwinish:bar_part_of_window", "Whether the bar is a part of the window (reserves space)", true);
    g_pGlobalState->config.barPrecedenceOverBorder =
        makeShared<Config::Values::CBoolValue>("plugin:kwinish:bar_precedence_over_border", "Whether the bar is before, or after the border", true);
    g_pGlobalState->config.barButtonsAlignment = makeShared<Config::Values::CStringValue>("plugin:kwinish:bar_buttons_alignment", "Alignment of the bar buttons", "right");
    g_pGlobalState->config.barPadding          = makeShared<Config::Values::CIntValue>("plugin:kwinish:bar_padding", "Padding of the bar", 7);
    g_pGlobalState->config.barButtonPadding    = makeShared<Config::Values::CIntValue>("plugin:kwinish:bar_button_padding", "Padding of the bar buttons", 5);
    g_pGlobalState->config.enabled             = makeShared<Config::Values::CBoolValue>("plugin:kwinish:enabled", "Whether bars are enabled", true);
    g_pGlobalState->config.iconOnHover         = makeShared<Config::Values::CBoolValue>("plugin:kwinish:icon_on_hover", "Whether to use an icon on hover of the buttons", false);
    g_pGlobalState->config.floatingOnly        = makeShared<Config::Values::CBoolValue>("plugin:kwinish:floating_only", "Draw bars on floating windows only", false);
    g_pGlobalState->config.showIcon            = makeShared<Config::Values::CBoolValue>("plugin:kwinish:show_icon", "Show the app icon at the left of the bar", true);
    g_pGlobalState->config.defaultButtons      = makeShared<Config::Values::CBoolValue>("plugin:kwinish:default_buttons", "Install the default buttons (pin, minimize, maximize, close) when none are configured", true);
    g_pGlobalState->config.buttonHoverColor    = makeShared<Config::Values::CColorValue>("plugin:kwinish:button_hover_color", "Background of a hovered button", 0x33ffffff);
    g_pGlobalState->config.pinnedColor         = makeShared<Config::Values::CColorValue>("plugin:kwinish:pinned_color", "Background of the pin button while the window is pinned", 0x667aa2f7);
    g_pGlobalState->config.onDoubleClick       = makeShared<Config::Values::CStringValue>("plugin:kwinish:on_double_click", "Action on double click of the bar (same names as button actions)", "maximize");

    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barColor);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.textColor);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.inactiveButtonColor);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barHeight);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barTextSize);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barTextWeight);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barTitleEnabled);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barBlur);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barTextFont);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barTextAlign);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barPartOfWindow);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barPrecedenceOverBorder);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barButtonsAlignment);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barPadding);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.barButtonPadding);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.enabled);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.iconOnHover);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.onDoubleClick);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.floatingOnly);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.showIcon);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.defaultButtons);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.buttonHoverColor);
    HyprlandAPI::addConfigValueV2(PHANDLE, g_pGlobalState->config.pinnedColor);

    if (Config::mgr()->type() == Config::CONFIG_LEGACY)
        HyprlandAPI::addConfigKeyword(PHANDLE, "plugin:kwinish:button", onNewButton, Hyprlang::SHandlerOptions{});
    else
        HyprlandAPI::addLuaFunction(PHANDLE, "kwinish", "add_button", ::newLuaButton);
    static auto P6 = Event::bus()->m_events.window.floating.listen([&](PHLWINDOW w) {
        // floating_only: a bar appears or disappears when the window changes mode
        for (auto& b : g_pGlobalState->bars)
            if (b && b->getOwner() == w)
                b->onFloatingChanged();
    });
    static auto P4 = Event::bus()->m_events.config.preReload.listen([&] { onPreConfigReload(); });
    static auto P5 = Event::bus()->m_events.config.reloaded.listen([&] { onConfigReloaded(); });

    // add deco to existing windows
    for (auto& w : Desktop::windowState()->windows()) {
        if (w->isHidden() || !w->m_isMapped)
            continue;

        onNewWindow(w);
    }

    HyprlandAPI::reloadConfig();

    return {"kwinish-bars", "Title bars for Kwinish (fork of hyprbars).", "lhen, Vaxry (hyprbars)", "0.1"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    for (auto& m : State::monitorState()->monitors())
        m->m_scheduledRecalc = true;

    g_pHyprRenderer->m_renderPass.removeAllOfType("CBarPassElement");

    Desktop::Rule::windowEffects()->unregisterEffect(g_pGlobalState->barColorRuleIdx);
    Desktop::Rule::windowEffects()->unregisterEffect(g_pGlobalState->titleColorRuleIdx);
    Desktop::Rule::windowEffects()->unregisterEffect(g_pGlobalState->nobarRuleIdx);
}
