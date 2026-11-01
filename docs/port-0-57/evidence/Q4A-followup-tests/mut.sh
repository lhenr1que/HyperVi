#!/usr/bin/env bash
# Q4-A model mutations: each edits a copy of q4model.hpp / q4test.cpp (persistent variant lives in q4test.cpp).
# "killed" = builds (GCC, ASan/UBSan) and the suite fails; a survivor is reported with the reason it is expected
# (redundant under the stated premises) or flagged UNEXPECTED.
set -u
P=$(cd "$(dirname "$0")" && pwd); M=$P/mut; rm -rf "$M"; mkdir -p "$M"
HU="$(pkg-config --cflags --libs hyprutils)"
FAILN=0
while IFS='~' read -r NAME FILE EXPECT FIND REPL; do
    [ -z "$NAME" ] && continue
    D=$M/$NAME; mkdir -p "$D"; cp "$P/q4model.hpp" "$P/q4test.cpp" "$P/review_cases.hpp" "$D/"
    FF="$(printf '%b' "$FIND")" RR="$(printf '%b' "$REPL")" perl -0pi -e 's/\Q$ENV{FF}\E/$ENV{RR}/' "$D/$FILE"
    cmp -s "$P/$FILE" "$D/$FILE" && { echo "FAIL - $NAME did not apply"; FAILN=1; continue; }
    if ! g++ -std=c++2b -g -fsanitize=address,undefined -fno-sanitize-recover=all "$D/q4test.cpp" -o "$D/t" $HU >"$D/build.log" 2>&1; then echo "FAIL - $NAME does not build"; FAILN=1; continue; fi
    "$D/t" >"$D/run.txt" 2>&1; RC=$?
    FIRST=$(grep -m1 -E 'FALHOU|ERROR: AddressSanitizer|runtime error' "$D/run.txt" | cut -c1-110)
    if [ "$EXPECT" = kill ]; then
        if [ $RC -ne 0 ] && grep -q FALHOU "$D/run.txt" && ! grep -q 'AddressSanitizer' "$D/run.txt"; then echo "OK   - $NAME killed: $FIRST"; else echo "FAIL - $NAME expected kill (rc=$RC) $FIRST"; FAILN=1; fi
    else
        if [ $RC -eq 0 ]; then echo "OK   - $NAME survives as expected: $EXPECT"; else echo "NOTE - $NAME expected redundant ($EXPECT) but killed: $FIRST"; fi
    fi
done <<'EOF'
q01~q4model.hpp~kill~        if (g_dc->target())\n            return ALREADY;                       // anyone's drag~        if (false)\n            return ALREADY;                       // anyone's drag
q02~q4model.hpp~kill~        if (unloading || inOwnEnd || inBegin)~        if (unloading || inBegin)
q03~q4model.hpp~kill~        if (unloading || inOwnEnd || inBegin)~        if (inOwnEnd || inBegin)
q04~q4model.hpp~kill~                disarm();~                ownedAt.reset();
q05~q4model.hpp~kill~        disarm();        // listener destroyed before the module is unmapped~        (void)0;
q06~q4model.hpp~kill~            g_dc->dragBegin(t, m);\n            return g_dc->target() ? STARTED : REFUSED;~            return REFUSED;
q07~q4model.hpp~redundant (endIfOurs checks owns() itself)~        if (owns())\n            (void)endIfOurs();\n        disarm();~        (void)endIfOurs();\n        disarm();
q08~q4model.hpp~kill~    bool owns() const { return ownedAt && *ownedAt == generation && !ownedTarget.expired() && g_dc->target() == ownedTarget.lock() && listener; }~    bool owns() const { return (bool)g_dc->target(); }
q09~q4model.hpp~redundant under premises (the handler disarms on every `ended`)~*ownedAt == generation && ~
q10~q4model.hpp~redundant (target equality already fails when our target expired)~!ownedTarget.expired() && ~
q11~q4model.hpp~redundant under A3 (no unguarded re-begin; `ended` disarms otherwise)~ && g_dc->target() == ownedTarget.lock()~
q12~q4model.hpp~redundant (listener is null exactly when disarmed)~ && listener; }~; }
q13~q4model.hpp~kill~generation != G0 || ~
q14~q4test.cpp~kill~    bool owns() const { return ownedAt && *ownedAt == generation && !ownedTarget.expired() && g_dc->target() == ownedTarget.lock(); }~    bool owns() const { return ownedAt && !ownedTarget.expired() && g_dc->target() == ownedTarget.lock(); }
q15~q4test.cpp~kill~        if (generation != G0 || g_dc->target() != t) return REFUSED;~        if (g_dc->target() != t) return REFUSED;
q16~q4model.hpp~kill~if (unloading || inOwnEnd || inBegin)~if (unloading || inOwnEnd)
q17~q4model.hpp~kill~if (token != acquisition) return;~if (false) return;
EOF
echo "result: $([ $FAILN = 0 ] && echo PASS || echo FAIL)"; exit $FAILN
