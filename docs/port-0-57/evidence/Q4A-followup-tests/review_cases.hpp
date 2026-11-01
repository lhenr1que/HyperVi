#include "q4model.hpp"
#include <cstdio>
namespace ReviewCases {
static int failures=0;
static void ck(bool b,const char* name) { std::printf("%s - %s\n",b?"OK":"FALHOU",name); failures+=!b; }
static SPT mk(const char* n) { auto x=makeShared<STarget>(); x->name=n; return x; }
inline int reviewCases() {
    for (bool same:{false,true}) {
        CDragModel dc; g_dc=&dc; COwnership p; auto A=mk("A"),B=same?A:mk("B");
        A->mapped=false;
        eBeginResult inner=ALREADY;
        bool once=false;
        auto earlier=dc.m_events.ended.listen([&] {
            if(once) return;
            once=true;
            B->mapped=true;
            inner=p.begin(B,M_MOVE);
        });
        auto outer=p.begin(A,M_MOVE);
        std::printf("R1 same=%d outer=%d inner=%d target=%s owns=%d\n",same,outer,inner,dc.target()?dc.target()->name.c_str():"none",p.owns());
        ck(inner==REFUSED,same?"R1.same: nested begin refused while outer begin in progress":"R1.other: nested begin refused while outer begin in progress");
        ck(outer==REFUSED && !dc.target() && !p.listener,"R1: failed outer acquisition has no owned or orphan session");
    }
    {
        CDragModel dc; g_dc=&dc; COwnership p; auto A=mk("A"),X=mk("X"),B=mk("B");
        int level=0;
        eBeginResult inner=REFUSED;
        auto earlier=dc.m_events.ended.listen([&] {
            if(level==0) { level=1; dc.dragBegin(X,M_MOVE); dc.dragEnd(); }
            else if(level==1) { level=2; inner=p.begin(B,M_MOVE); }
        });
        p.begin(A,M_MOVE);
        dc.dragEnd();
        std::printf("R2 inner=%d target=%s owns=%d handlerRuns=%d\n",inner,dc.target()?dc.target()->name.c_str():"none",p.owns(),p.handlerRuns);
        ck(inner==STARTED && dc.target()==B && p.owns(),"R2: nested snapshots cannot invalidate newer acquisition");
        ck(p.handlerRuns==0,"R2: obsolete callback tokens ignored in both snapshots");
        earlier.reset(); p.endIfOurs();
        ck(!dc.target() && !p.listener,"R2: newer acquisition ends normally");
    }
    {
        CDragModel dc; g_dc=&dc; COwnership p; auto A=mk("A"),X=mk("X");
        p.begin(A,M_MOVE);
        dc.onFocusDuringEnd=[&] { dc.onFocusDuringEnd=nullptr; dc.dragBegin(X,M_MOVE); };
        dc.dragEnd();
        std::printf("R3 target=%s mode=%d pluginAdditionalEnd=%d\n",dc.target()?dc.target()->name.c_str():"none",dc.m_mode,p.endIfOurs());
        ck(dc.target()==X && !p.endIfOurs(),"R3: ownership policy issues no additional end on X");
#ifdef REQUIRE_FULL_INTACT
        ck(dc.m_mode==M_MOVE,"R3: full-intact claim requires MOVE mode (expected red: host clobbers mode)");
#else
        ck(dc.m_mode==M_INVALID,"R3 documented host limit: enclosing end resets new drag mode to INVALID");
#endif
    }
    // R4: target reset and ended emission are NOT atomic. A third party can re-begin on the SAME target,
    // then a plugin end entry sees the old ownership before ended invalidates it. Do NOT assume A3 excludes this.
    for (bool pluginInitiatesEnd:{false,true}) {
        CDragModel dc; g_dc=&dc; COwnership p; auto A=mk("A");
        bool additionalEnd=false;
        p.begin(A,M_MOVE);
        dc.onFocusDuringEnd=[&] {
            dc.onFocusDuringEnd=nullptr;
            dc.dragBegin(A,M_MOVE); // target is already empty: obeys A3
            additionalEnd=p.endIfOurs();
        };
        if(pluginInitiatesEnd) p.endIfOurs(); else dc.dragEnd();
        std::printf("R4 pluginInitiatesEnd=%d additionalEnd=%d target=%s emissions=%d\n",
                    pluginInitiatesEnd,additionalEnd,dc.target()?dc.target()->name.c_str():"none",dc.endedEmissions);
        ck(!additionalEnd && dc.target()==A,
           pluginInitiatesEnd?"R4.own: no extra plugin end of same-target third-party drag before outer ended":"R4.host: no extra plugin end of same-target third-party drag before outer ended");
    }
    std::printf("review regressions: %d failures\n",failures);
    return failures;
}
} // namespace ReviewCases
