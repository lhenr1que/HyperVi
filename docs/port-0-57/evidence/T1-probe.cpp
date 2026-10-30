// T1 probe (group M) — installed Hyprland 0.57.0 headers, Makefile flags, -fsyntax-only.
// Default: the 0.57 forms used by T1 compile and keep the 0.56.2 semantics.
// -DPROBE_NEG=1: the 0.56.2 Shift test (barDeco.cpp:486/605) must fail to compile.
// -DPROBE_NEG=2: Config::CONFIG_LEGACY (main.cpp:436/440) must fail to compile.
// -DPROBE_NEG=3: reading the 0.57 mask as uint32_t (the 0.56.2 return type) must fail.
// -DPROBE_MUT=1..5: wrong forms of the Shift test; each must trip a static_assert.
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/config/ConfigManager.hpp>

#include <cstdint>
#include <type_traits>

using Input::eKeyboardModifiers;
using Input::ModifierMask;

// --- 0.57 types -----------------------------------------------------------------------
static_assert(std::is_same_v<decltype(std::declval<CInputManager&>().getModsFromAllKBs()), ModifierMask>);
static_assert(std::is_same_v<std::underlying_type_t<eKeyboardModifiers>, uint8_t>);
static_assert(!std::is_convertible_v<ModifierMask, uint32_t>, "no implicit integer view of the mask");
static_assert(!std::is_convertible_v<ModifierMask, bool>, "bool view is explicit: only contextual use");

// --- bit values unchanged from 0.56.2 (src/devices/IKeyboard.hpp:14,16) ----------------
static_assert(static_cast<uint8_t>(eKeyboardModifiers::HL_MODIFIER_SHIFT) == (1 << 0));
static_assert(static_cast<uint8_t>(eKeyboardModifiers::HL_MODIFIER_CTRL) == (1 << 2));

// --- 0.56.2 Config type: CONFIG_LEGACY = 0, CONFIG_LUA = 1; 0.57 keeps only CONFIG_LUA ----
static_assert(Config::CONFIG_LUA == 0);

// --- the T1 Shift test, exactly as written in barDeco.cpp ------------------------------
#ifndef PROBE_MUT
#define PROBE_MUT 0
#endif
constexpr bool kwShift(ModifierMask mods) {
#if PROBE_MUT == 1
    if (mods & eKeyboardModifiers::HL_MODIFIER_CTRL) // wrong modifier
#elif PROBE_MUT == 2
    if (mods & eKeyboardModifiers::HL_MODIFIER_CAPS) // neighbouring bit
#elif PROBE_MUT == 3
    if (!(mods & eKeyboardModifiers::HL_MODIFIER_SHIFT)) // inverted
#elif PROBE_MUT == 4
    if (mods == ModifierMask{eKeyboardModifiers::HL_MODIFIER_SHIFT}) // equality: Shift alone only
#elif PROBE_MUT == 5
    if (mods & eKeyboardModifiers::HL_MODIFIER_NONE) // never
#else
    if (mods & eKeyboardModifiers::HL_MODIFIER_SHIFT)
#endif
        return true;
    return false;
}

// 0.56.2 oracle: `getModsFromAllKBs() & HL_MODIFIER_SHIFT` on a uint32_t, HL_MODIFIER_SHIFT = 1 << 0.
constexpr bool oldShift(uint32_t mods) {
    return (mods & (1u << 0)) != 0;
}

// Exhaustive over the 8-bit mask (all reachable values of the 0.57 type).
constexpr bool sameForAll() {
    for (unsigned i = 0; i < 256; ++i)
        if (kwShift(ModifierMask{static_cast<uint8_t>(i)}) != oldShift(i))
            return false;
    return true;
}
static_assert(sameForAll(), "T1 Shift test differs from 0.56.2 on some modifier mask");
static_assert(kwShift(eKeyboardModifiers::HL_MODIFIER_SHIFT | eKeyboardModifiers::HL_MODIFIER_CTRL), "Shift+Ctrl counts as Shift");
static_assert(!kwShift(eKeyboardModifiers::HL_MODIFIER_CTRL), "Ctrl alone is not Shift");
static_assert(!kwShift(eKeyboardModifiers::HL_MODIFIER_NONE));

// The production expression binds against the real global and member (not evaluated).
[[maybe_unused]] static bool prodShape() {
    if (g_pInputManager->getModsFromAllKBs() & Input::eKeyboardModifiers::HL_MODIFIER_SHIFT)
        return true;
    return Config::mgr()->type() == Config::CONFIG_LUA;
}

#if PROBE_NEG == 1
[[maybe_unused]] static bool negOldShift() {
    return static_cast<bool>(g_pInputManager->getModsFromAllKBs() & HL_MODIFIER_SHIFT);
}
#elif PROBE_NEG == 2
[[maybe_unused]] static bool negLegacy() {
    return Config::mgr()->type() == Config::CONFIG_LEGACY;
}
#elif PROBE_NEG == 3
[[maybe_unused]] static uint32_t negIntMask() {
    uint32_t m = g_pInputManager->getModsFromAllKBs();
    return m;
}
#endif
