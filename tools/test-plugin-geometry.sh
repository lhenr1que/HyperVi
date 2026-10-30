#!/usr/bin/env bash
# Host-free geometry contract; no Hyprland, GL, plugin loading or installation.
# CXX selects the compiler (default g++); run once per compiler.
set -u
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
t=$(mktemp -d "${TMPDIR:-${XDG_CACHE_HOME:-$HOME/.cache}}/kwinish-test-geometry.XXXXXXXX") || exit 1
trap 'rm -rf -- "$t"' EXIT
cxx=${CXX:-g++}
command -v "$cxx" >/dev/null || { echo "FALHOU - falta o compilador $cxx"; exit 1; }
command -v perl >/dev/null || { echo "FALHOU - falta perl (mutacoes do header)"; exit 1; }
flags=(-std=c++2b -Wall -Wextra -Werror)
"$cxx" "${flags[@]}" -I "$root/plugin" "$root/plugin/test_geometry.cpp" -o "$t/test_geometry" || { echo "FALHOU - compilar plugin/test_geometry.cpp"; exit 1; }
"$t/test_geometry" || exit 1

# Fault injection inside the TEST executable. Each variant runs the SAME
# assertions and must exit 1 with its OWN expected assertion; a signal, a
# compile error, a CLI error or a different assertion is not evidence.
negative() { # mode expected-substring
    "$t/test_geometry" --negative "$1" >"$t/negative.log" 2>&1
    local status=$? message
    IFS= read -r message <"$t/negative.log" || message=''
    if [[ $status -ne 1 || $message != "FALHOU - $2"* ]]; then
        printf 'FALHOU - negativo %s nao detectado pela assertiva esperada "%s" (exit %s): %s\n' "$1" "$2" "$status" "$message"
        exit 1
    fi
    printf 'OK - negativo %s detectado: %s\n' "$1" "$message"
}
negative 1 'P1 literal numeric oracle'
negative 2 'P1 literal numeric oracle'
negative 3 'P1 literal numeric oracle'
negative 4 'P1 literal numeric oracle'
negative 5 'P1 literal numeric oracle'
negative 6 'P2 logical F / bottom extension / AA'
negative 7 'P2 logical F / bottom extension / AA'
negative 8 'P3 fixed expand(10) cannot cover large bottom extension'
negative 9 'P4 fractional union preserves logical edges'
negative 10 'P5 all components divide by scale exactly once'
negative 11 'P1 literal numeric oracle'
negative 12 'P1 literal numeric oracle'
negative 13 'P5 all components divide by scale exactly once'

# Real mutations of geometry.hpp, applied to a scratch copy only. Each one must
# change the file, compile (or, for COMPILE, fail with a static_assert) and be
# rejected by its OWN assertion, never by an unrelated failure.
mutant() { # name find replace expected|COMPILE
    local dir="$t/mut" out
    rm -rf "$dir"; mkdir "$dir"
    cp "$root/plugin/test_geometry.cpp" "$dir/"
    A=$2 B=$3 perl -0777 -pe 'BEGIN { $a = $ENV{A}; $b = $ENV{B} } s/\Q$a\E/$b/' "$root/plugin/geometry.hpp" >"$dir/geometry.hpp"
    if cmp -s "$root/plugin/geometry.hpp" "$dir/geometry.hpp"; then
        printf 'FALHOU - mutacao "%s" nao alterou o header (padrao desatualizado)\n' "$1"; exit 1
    fi
    if [[ $4 == COMPILE ]]; then
        out=$("$cxx" "${flags[@]}" -I "$dir" "$dir/test_geometry.cpp" -o "$dir/t" 2>&1) && {
            printf 'FALHOU - mutacao "%s" compilou; esperava static_assert\n' "$1"; exit 1; }
        [[ $out == *'static assertion failed'* ]] || { printf 'FALHOU - mutacao "%s" falhou sem static_assert\n' "$1"; exit 1; }
        printf 'OK - mutacao "%s" rejeitada por static_assert\n' "$1"
        return
    fi
    "$cxx" "${flags[@]}" -I "$dir" "$dir/test_geometry.cpp" -o "$dir/t" >"$dir/cc.log" 2>&1 || {
        printf 'FALHOU - mutacao "%s" nao compilou (erro de compilacao nao e deteccao)\n' "$1"; exit 1; }
    out=$("$dir/t" 2>&1); local status=$?
    if [[ $status -ne 1 || $out != "FALHOU - $4"* ]]; then
        printf 'FALHOU - mutacao "%s" nao detectada pela assertiva esperada "%s" (exit %s): %s\n' "$1" "$4" "$status" "$out"
        exit 1
    fi
    printf 'OK - mutacao "%s" detectada: %s\n' "$1" "$out"
}
X='+ w.x + f.x - m.x) * scale;'
mutant 'W duas vezes'            "$X" '+ w.x + w.x + f.x - m.x) * scale;' 'P1 literal numeric oracle'
mutant 'F duas vezes'            "$X" '+ w.x + f.x + f.x - m.x) * scale;' 'P1 literal numeric oracle'
mutant 'M omitido'               "$X" '+ w.x + f.x) * scale;' 'P1 literal numeric oracle'
mutant 'M com sinal errado'      "$X" '+ w.x + f.x + m.x) * scale;' 'P1 literal numeric oracle'
mutant 'escala dupla'            "$X" '+ w.x + f.x - m.x) * scale * scale;' 'P1 literal numeric oracle'
mutant 'arredonda w/h isolados'  'std::round(x + d.w * scale - rx), std::round(y + d.h * scale - ry)}' 'std::round(d.w * scale), std::round(d.h * scale)}' 'P1 literal numeric oracle'
mutant 'largura = altura'        'std::round(x + d.w * scale - rx), std::round(y + d.h * scale - ry)}' 'std::round(y + d.h * scale - ry), std::round(y + d.h * scale - ry)}' 'P1 literal numeric oracle'
mutant 'bbox sem F'              'const LogicalBox b{d.x + e.x + w.x + f.x - m.x - aaMargin,' 'const LogicalBox b{d.x + e.x + w.x - m.x - aaMargin,' 'P2 logical F / bottom extension / AA'
mutant 'bbox sem extensao'       'd.h + extBottom + 2 * aaMargin}' 'd.h + 2 * aaMargin}' 'P2 logical F / bottom extension / AA'
mutant 'bbox em unidade fisica'  'd.w + 2 * aaMargin, d.h' '(d.w + 2 * aaMargin) * 1.5, d.h' 'P2 logical F / bottom extension / AA'
mutant 'damage so do novo'       'const double x = std::min(prev.x, next.x), y = std::min(prev.y, next.y);' 'const double x = next.x, y = next.y;' 'P4 fractional union preserves logical edges'
mutant 'damage sem nextafter'    'if (x + w < right) w = std::nextafter(w, INFINITY);' '' 'P4 fractional union preserves logical edges'
mutant 'preview fisico como logico' 'LogicalBox b{physical.x / scale, physical.y / scale, physical.w / scale, physical.h / scale};' 'LogicalBox b{physical.x, physical.y, physical.w, physical.h};' 'P5 all components divide by scale exactly once'
mutant 'preview dimensoes sem escala' 'physical.w / scale, physical.h / scale};' 'physical.w, physical.h};' 'P5 all components divide by scale exactly once'
mutant 'preview sem alargar origem'  'origin = std::nextafter(origin, -INFINITY);' 'origin += 0;' 'P5 ulp-adjacent origins and edges preserve strict coverage'
mutant 'preview sem alargar extensao' 'extent = std::nextafter(boundary - origin, INFINITY);' '(void)boundary;' 'P5 fractional right edge above integer (review regression)'
mutant 'preview borda sem nextafter' 'const double boundary = std::nextafter(end / scale, INFINITY);' 'const double boundary = end / scale;' 'P5 ulp-adjacent origins and edges preserve strict coverage'
mutant 'resolvido: physicalLocal sem teste' 'if (!detail::resolved(x, d.w * scale) || !detail::resolved(y, d.h * scale)) return std::nullopt;' '' 'P1 physicalLocal rejects an extent absorbed by the origin (was width -1)'
mutant 'resolvido: bbox sem teste'  '        return detail::boxOk(b) ? std::optional<LogicalBox>(b) : std::nullopt;
    }

    // Both boxes' '        return b;
    }

    // Both boxes' 'P2 logicalBBox rejects an extent absorbed by the origin'
mutant 'saida: union sem pos-teste' 'return detail::boxOk(result) ? std::optional<LogicalBox>(result) : std::nullopt;' 'return result;' 'P4 unionDamage refuses a union wider than the box domain'
mutant 'saida: preview sem pos-teste' '// left the box domain is refused instead of returned.
        return detail::boxOk(b) ? std::optional<LogicalBox>(b) : std::nullopt;' '// left the box domain is refused instead of returned.
        return b;' 'P5 previewBBox refuses a result whose extent collapsed by division'
mutant 'resolvido: union sem teste' 'resolved(b.x, b.w) && resolved(b.y, b.h);' 'true;' 'P2 logicalBBox rejects an extent absorbed by the origin'
mutant 'resolvido: preview sem teste' '!detail::resolved(physical.x, physical.w) || !detail::resolved(physical.y, physical.h) ||' '' 'P5 previewBBox rejects an extent absorbed by a negative origin'
mutant 'resolvido: so eixo x'       'resolved(b.x, b.w) && resolved(b.y, b.h);' 'resolved(b.x, b.w);' 'P2 logicalBBox rejects an extent absorbed by the origin'
mutant 'resolvido: aceita igualdade' 'return x + w > x; }' 'return x + w >= x; }' 'P1 physicalLocal rejects an extent absorbed by the origin (was width -1)'
# Domain (P3): each guard removed or weakened on its own.
mutant 'dominio: within sem limite'        'return std::fabs(v) <= limit; }' 'return std::fabs(v) <= limit || limit > 0; }' 'P3 original counterexample 2^53 rejected by physicalLocal'
mutant 'dominio: limite aberto'            'return std::fabs(v) <= limit; }' 'return std::fabs(v) < limit; }' 'P3 largest in-domain analogue of the counterexample covered'
mutant 'dominio: sem fabs (so positivo)'   'return std::fabs(v) <= limit; }' 'return v <= limit; }' 'P3 physicalLocal rejects non-finite/overflow/out-of-domain field'
mutant 'dominio: comprimento aceita zero'  'return v > 0 && v <= limit; }' 'return v >= 0 && v <= limit; }' 'P3 logicalBBox rejects zero/negative length even when AA/extension would widen it'
mutant 'dominio: comprimento aceita negativo' 'return v > 0 && v <= limit; }' 'return v <= limit; }' 'P3 logicalBBox rejects zero/negative length even when AA/extension would widen it'
mutant 'dominio: comprimento sem teto'    'return v > 0 && v <= limit; }' 'return v > 0 && limit > 0; }' 'P3 physicalLocal rejects non-finite/overflow/out-of-domain field'
mutant 'dominio: escala sem minimo'        's >= kMinScale && s <= kMaxScale' 's <= kMaxScale' 'P3 physicalLocal rejects zero/negative length or scale below minimum'
mutant 'dominio: escala sem maximo'        's >= kMinScale && s <= kMaxScale' 's >= kMinScale' 'P3 physicalLocal rejects non-finite/overflow/out-of-domain field'
mutant 'dominio: escala minimo aberto'     's >= kMinScale && s <= kMaxScale' 's > kMinScale && s <= kMaxScale' 'P3 physicalLocal accepts the closed domain limit'
mutant 'dominio: y do ponto sem teste'     'within(p.x, kMaxLogical) && within(p.y, kMaxLogical)' 'within(p.x, kMaxLogical)' 'P3 physicalLocal rejects non-finite/overflow/out-of-domain field'
mutant 'dominio: D.h sem teste'            '&&
                length(d.h, kMaxLogical)' '&&
                true' 'P3 physicalLocal rejects non-finite/overflow/out-of-domain field'
mutant 'dominio: extBottom aceita negativo' 'return v >= 0 && v <= limit; }' 'return v <= limit; }' 'P3 logicalBBox rejects bad extBottom'
mutant 'dominio: extBottom sem teto'       'return v >= 0 && v <= limit; }' 'return v >= 0 && limit > 0; }' 'P3 logicalBBox rejects bad extBottom'
mutant 'dominio: aaMargin sem teste'       '!detail::nonneg(aaMargin, kMaxLogical))' 'false)' 'P3 logicalBBox rejects bad aaMargin'
mutant 'dominio: unionDamage sem teste'    'if (!detail::boxOk(prev) || !detail::boxOk(next)) return std::nullopt;' '' 'P4 unionDamage rejects an absorbed extent in the second operand'
mutant 'dominio: unionDamage so prev'      '!detail::boxOk(prev) || !detail::boxOk(next)' '!detail::boxOk(prev)' 'P4 unionDamage rejects an absorbed extent in the second operand'
mutant 'dominio: previewBBox sem teto fisico' '!detail::within(physical.x, kMaxPhysical) || !detail::within(physical.y, kMaxPhysical) ||
            !detail::length(physical.w, kMaxPhysical) || !detail::length(physical.h, kMaxPhysical) ||
            ' '' 'P5 previewBBox rejects bad physical component'
mutant 'dominio: previewBBox sem escala'   '||
            !detail::scaleOk(scale))' ')' 'P5 previewBBox rejects bad scale'
mutant 'dominio: kMaxLogical = 2^53'       'kMaxLogical = 0x1p31' 'kMaxLogical = 0x1p53' COMPILE
mutant 'dominio: kMaxBox = 2^53'           'kMaxBox = 0x1p40' 'kMaxBox = 0x1p53' COMPILE
mutant 'dominio: kMaxScale = 2^20'         'kMaxScale = 0x1p4' 'kMaxScale = 0x1p20' COMPILE
echo "OK - $cxx: suite, 13 negativos e todas as mutacoes rejeitados pela assertiva esperada"
