#!/usr/bin/env bash
# T7: inspeção ELF estática de um kwinish-bars.so (nada é carregado: só readelf/nm; ldd só no Hyprland instalado).
# uso: elf.sh <so>. Exit 0 = tudo passou. Critérios: RISKS-TESTS.md B1-S3 (S5) e LEDGER.md H-DTOR, mais
#  - exporta os nomes que o host procura com dlsym (PluginAPI.hpp:102–123: PLUGIN_* são macros para pluginAPIVersion,
#    pluginInit, pluginExit; as mesmas strings estão no binário do host);
#  - todo símbolo indefinido forte é definido pelo /usr/bin/Hyprland ou por uma biblioteca que ele ou o .so carregam.
# Limite: resolver por nome não prova ABI compatível nem carga bem-sucedida (isso é T8, ao vivo).
set -u
so=$1; hl=${HYPRLAND_BIN:-/usr/bin/Hyprland}; fails=0
t=$(mktemp -d); trap 'rm -rf "$t"' EXIT
ok() { echo "ok - $1"; }; bad() { echo "FALHOU - $1"; fails=$((fails + 1)); }

echo "# $so ($(sha256sum "$so" | cut -c1-16)); host: $("$hl" --version | head -n 1)"
readelf -d "$so" > "$t/dyn"
grep -q NODELETE "$t/dyn" && bad "FLAGS_1 NODELETE presente (dlclose não descarrega)" || ok "sem NODELETE"
grep -E 'NEEDED' "$t/dyn" | sed 's/^.*\[/    NEEDED [/'

nm -D "$so" > "$t/nm"
if grep -E '^[0-9a-f]+ u ' "$t/nm" > "$t/uniq"; then bad "símbolos GNU_UNIQUE (impedem dlclose):"; sed 's/^/    /' "$t/uniq" | head; else ok "sem símbolos GNU_UNIQUE"; fi
for s in pluginAPIVersion pluginInit pluginExit; do
  strings "$hl" | grep -qx "$s" || bad "o host não procura $s"
  grep -qE " T ${s}\$" "$t/nm" && ok "exporta $s" || bad "não exporta $s"
done

# tudo que o processo do compositor terá carregado: o binário, as libs dele e as do .so (sem versão de símbolo)
{ echo "$hl"; ldd "$hl" | awk '/=> \//{print $3}'; } | sort -u > "$t/libs"
grep NEEDED "$t/dyn" | sed 's/.*\[\(.*\)\]/\1/' | while read -r n; do ldconfig -p | awk -v n="$n" '$1==n{print $NF; exit}'; done >> "$t/libs"
sort -u "$t/libs" | while read -r l; do nm -D --defined-only "$l" 2>/dev/null; done | awk '{print $3}' | sed 's/@.*//' | sort -u > "$t/defined"
grep -E '^ +U ' "$t/nm" | awk '{print $2}' | sed 's/@.*//' | sort -u > "$t/undef"
comm -23 "$t/undef" "$t/defined" > "$t/missing"
n=$(wc -l < "$t/undef"); m=$(wc -l < "$t/missing")
if [ "$m" = 0 ]; then ok "$n símbolos indefinidos fortes, todos definidos no host ou nas libs carregadas ($(sort -u "$t/libs" | wc -l) objetos)"
else bad "$m de $n símbolos indefinidos sem definição:"; c++filt < "$t/missing" | sed 's/^/    /' | head -n 40; fi

# vtables/typeinfo dos passes próprios: residem no módulo (Q1 exige que nenhum passe sobreviva ao unload)
for c in CBarPassElement CPreviewPassElement; do
  v=$(grep -cE " [VvWwDdRr] _ZT[VI]${#c}${c}\$" "$t/nm")
  [ "$v" -gt 0 ] && echo "    info: vtable/typeinfo de $c definidas no módulo ($v)" || echo "    info: vtable/typeinfo de $c não exportadas (locais ao módulo)"
done

[ $fails = 0 ] && echo "elf: tudo passou" || { echo "elf: $fails falha(s)"; exit 1; }
