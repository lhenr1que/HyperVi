// SPDX-License-Identifier: MIT
// Part of HyperVi (Copyright (c) 2026 lhenr1que).
// plugin/hostDrag.hpp is not included by any plugin source while the title-bar drag is off (alpha-0), so the build no
// longer checks it: tools/test-plugin-hostdrag.sh compiles this file (syntax only) against the installed headers.
#include "hostDrag.hpp"

#include <type_traits>

static_assert(std::is_same_v<decltype(&KwDrag::begin), KwDrag::eBeginResult (*)(eMouseBindMode)>);
