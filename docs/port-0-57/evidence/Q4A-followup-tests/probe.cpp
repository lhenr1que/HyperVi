// Q4-A: real installed 0.57 types, not the manual model.
#include "hostDrag.hpp"
#include <type_traits>
#include <utility>
using DC=Layout::Supplementary::CDragStateController;
using Signal=std::remove_reference_t<decltype(std::declval<DC&>().m_events.ended)>;
static_assert(std::is_same_v<Signal,CSignalT<>>);
static_assert(std::is_same_v<decltype(std::declval<Signal&>().listen([]{})),CHyprSignalListener>);
static_assert(std::is_same_v<decltype(&DC::dragEnd),bool (DC::*)()>);
static_assert(std::is_same_v<decltype(&Layout::CLayoutManager::endDragTarget),bool (Layout::CLayoutManager::*)()>);
static_assert(std::is_same_v<decltype(std::declval<DC&>().target()),SP<Layout::ITarget>>);
static_assert(std::is_same_v<decltype(std::declval<CHyprSignalListener&>().reset()),void>);
#if defined(NEG)
void invalidSessionId(DC& c) { (void)c.sessionId(); }
#endif
int main() {}
