#!/usr/bin/env bash
# T5 probe mutations: each reverts one ported form in a copy of plugin/ and must make run-probe.sh fail.
set -u
here=$(cd "$(dirname "$0")" && pwd -P); root=$(cd "$here/../../../.." && pwd -P); w=$(mktemp -d); trap 'rm -rf -- "$w"' EXIT
k=0 n=0
mut() { n=$((n+1)); rm -rf "$w/p"; cp -r "$root/plugin" "$w/p"; python3 -c "
import sys;p,o,r=sys.argv[1:];s=open(p).read();assert s.count(o)==1,o;open(p,'w').write(s.replace(o,r))" "$w/p/$2" "$3" "$4" || exit 3
  if bash "$here/run-probe.sh" "$w/p" >/dev/null 2>&1; then echo "SURVIVED $1"; else echo "killed $1"; k=$((k+1)); fi; }
mut M1-old-pass-draw BarPassElement.hpp 'draw(Render::CRenderContext& ctx) override' 'draw() override'
mut M2-old-deco-draw barDeco.hpp 'draw(Render::CRenderContext& ctx, PHLMONITOR, float const& a, const Render::SWindowRenderPresentation& presentation) override' 'draw(PHLMONITOR, float const& a)'
mut M3-preview-no-ctx PreviewPassElement.hpp 'needsLiveBlur(Render::CRenderContext&) override' 'needsLiveBlur() override'
mut M4-window-rounding-power barDeco.cpp '.roundingPower = PWINDOW->presentation().roundingPower(), .blur' '.roundingPower = m_pWindow->roundingPower(), .blur'
mut M5-old-stage-listener main.cpp 'const Event::SRenderStageEvent& e)' 'eRenderStage stage)'
mut M6-old-pass-queue barDeco.cpp 'g_pHyprRenderer->addPassElement(ctx, makeUnique<CBarPassElement>' 'g_pHyprRenderer->m_renderPass.add(makeUnique<CBarPassElement>'
echo "mutations: $k/$n killed"; [ $k = $n ]
