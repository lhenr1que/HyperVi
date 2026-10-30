All round-1 findings accepted; the document and probe were changed (current versions are below).

- Major 1 (Q4 ownership): target identity was replaced by session ownership. It is invalidated by the 0.57 `dragController()->m_events.ended` signal; probe K now asserts its type and the `listen` return type. Ownership is acquired only when the helper really called `beginDragTarget` AND `target()` is non-null afterwards. Your counterexample (plugin drag ended by the host, then a user bind drag on the same T) is now listed as test (d). New fact checked in host source: every host caller of `beginDragTarget` refuses when a target exists, so a new session always follows an `ended`. Callers: `Actions::mouse` (ConfigActions.cpp:1703), `CWindow::onMoveRequest`/`onResizeRequest` (Window.cpp:1861,1871, both with `exclusiveDeviceGrab=true`) and border resize (InputManager.cpp:876). Excerpt:
```
void CWindow::onMoveRequest() {
    if (!m_isMapped || isHidden() || g_layoutManager->dragController()->target())
        return;
    ...
    g_layoutManager->beginDragTarget(layoutTarget(), MBIND_MOVE, std::nullopt, true);
}
// InputManager.cpp:875-877
                if (!g_layoutManager->dragController()->target())
                    g_layoutManager->beginDragTarget(w->layoutTarget(), MBIND_RESIZE);
```
- Major 2 (reentry): the false claim was removed. Section 5 item 4 now lists the synchronous reentry routes and their requirements: `onInputOnDeco` via the CHyprBar vtable, `window.active` → `onFocusChanged`, decoration virtuals, and the `ended` listener. Requirements: no deferred work while unloading; step 2 runs before listener, decoration and timer removal. Proof that nothing runs after unmap is left to Q3.
- Minor: added the `m_draggingTiled` reset row (RD, with a group-drop test candidate) and the per-(button, pointer) orphan release.
- Minor (probe): description now matches what is proven. Added exact formal parameter types (function-pointer types and `static_cast` for overloads), the shortened calls used in the matrix, the `ended` type, and 3 new mutations: 36 OK, 26/26 mutants. Default values and the `m_passPressed` read are stated as coming from source text, not from the probe.
- Minor (K13): rewritten. Only the name lookup disappears; errors were already wrapped and discarded. Logging them is a separate policy change. `spawn` returns `optional<uint64_t>`.
- Minor (contract coverage): `docs/CONFIGURAVEL.md` reviewed (no drag, threshold, grab or minimize option). Conclusion reworded to "no BC identified in the reviewed sources".
- Also from the notes: K2 is now marked as inherited behaviour, and its backend resizing is DI with no client-visible effect demonstrated. K5 now keeps the focused-window selection and the no-window guard. K7 now requires keeping the `(int)` casts.
