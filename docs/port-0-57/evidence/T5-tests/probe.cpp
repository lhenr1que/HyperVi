// T5 probe: the render classes of the plugin are complete against the Hyprland 0.57 headers (no pure virtual left), and
// the tooltip listener takes the 0.57 stage event. Compiled with the plugin's flags; never linked or loaded.
#include "barDeco.hpp"
#include "BarPassElement.hpp"
#include "PreviewPassElement.hpp"
#include <hyprland/src/event/EventBus.hpp>
#include <type_traits>

static_assert(!std::is_abstract_v<CHyprBar>, "CHyprBar must implement the 0.57 IHyprWindowDecoration");
static_assert(!std::is_abstract_v<CBarPassElement>, "CBarPassElement must implement the 0.57 IPassElement");
static_assert(!std::is_abstract_v<CPreviewPassElement>, "CPreviewPassElement must implement the 0.57 IPassElement");
// the draw the host calls (IHyprWindowDecoration.hpp:50) reaches the bar's own override
static_assert(std::is_same_v<decltype(static_cast<void (CHyprBar::*)(Render::CRenderContext&, PHLMONITOR, float const&, const Render::SWindowRenderPresentation&)>(&CHyprBar::draw)),
                             void (CHyprBar::*)(Render::CRenderContext&, PHLMONITOR, float const&, const Render::SWindowRenderPresentation&)>);
int main() {}
