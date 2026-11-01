// Q4-A end policy: the observations the candidate relies on exist in the INSTALLED 0.57 headers with these exact types.
// Syntax-only against /usr/include/hyprland (no linking, no host). NEG must fail only on the missing session id.
#include "hostDrag.hpp"
#include <type_traits>
#include <utility>
using DC     = Layout::Supplementary::CDragStateController;
using Signal = std::remove_reference_t<decltype(std::declval<DC&>().m_events.ended)>;
static_assert(std::is_same_v<decltype(&DC::wasDraggingWindow), bool (DC::*)() const>);
static_assert(std::is_same_v<decltype(&DC::mode), eMouseBindMode (DC::*)() const>);
static_assert(std::is_same_v<decltype(std::declval<const DC&>().target()), SP<Layout::ITarget>>);
static_assert(std::is_same_v<Signal, CSignalT<>>); // no payload: `ended` does not say which session ended
static_assert(std::is_same_v<decltype(std::declval<Signal&>().listen([] {})), CHyprSignalListener>);
static_assert(std::is_same_v<decltype(&Layout::CLayoutManager::endDragTarget), bool (Layout::CLayoutManager::*)()>);
static_assert(std::is_same_v<decltype(KwDrag::begin(MBIND_MOVE)), KwDrag::eBeginResult>);
static_assert(KwDrag::BEGIN_REFUSED_BY_DECO != KwDrag::BEGIN_STARTED); // result-aware acquisition is expressible
#if defined(NEG)
void invalidSessionId(DC& c) { (void)c.sessionId(); }
#endif
int main() {}
