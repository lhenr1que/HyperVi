# Fluxo de coordenadas, offsets, clipping, damage e transformações (tarefa 1.4, parte B3)

Preparatório. Consolida em tabelas de contrato o que o código do baseline faz hoje e o que o 0.57 entrega; **não** repete a cadeia já documentada em `investigacao-b1-b3/B3-GEOMETRIA.md` §1–4. Legenda de evidência em [README](README.md).

## 1. Cadeia de referência (resumo, fonte: B3 §2)

`B_pixel = round( scale( D + E + W + F − M , s ) )` — `D` geometria relativa atribuída (lógico), `E` âncora `getEdgeDefinedPoint` (lógico global), `W`/`F` offsets de workspace e floating **do snapshot de apresentação**, `M` origem lógica do monitor de render, `s` escala do monitor. O host aplica depois `renderModif`, transformers e projeção do alvo; o plugin não os aplica.

Contratos de unidade do 0.57 [H]: `IPassElement::boundingBox/opaqueRegion(ctx)` = **lógico monitor-local** (`H:render/pass/PassElement.hpp`); `box` dos elementos nativos de rect/textura = **físico monitor-local**; `IHyprWindowDecoration::draw(ctx, monitor, a, presentation)` recebe `SWindowRenderPresentation` por valor com `workspaceOffset`, `floatingOffset`, `alpha`, `fadeAlpha`, `workspaceScene` etc. (`H:render/WindowRenderPresentation.hpp:9–19`).

## 2. Mapa baseline → port: de onde vem cada termo hoje

Cada consumidor do baseline calcula a posição à sua maneira. A coluna "Port" é a fonte alvo.

| Consumidor (baseline) | `D+E` | `W` | `F` | `−M` | `×s` | Extensão inferior | Port: fonte de `W`/`F` |
|---|---|---|---|---|---|---|---|
| `assignedBoxGlobal()` `barDeco.cpp:1151–1162` | sim | **vivo** `PWORKSPACE->m_renderOffset` (0 se `m_pinned`) | não | não | não | não | sem argumento de apresentação: ver G2 |
| Desenho `titleBarBox` `:1012–1019` | via `assignedBoxGlobal` | sim (herdado) | `PWINDOW->m_floatingOffset` | sim | sim, `round()` | `+ROUNDING*3` (lógico) | `presentation.workspaceOffset` / `.floatingOffset` |
| Caixa da janela (máscara) `:1028–1047` | posição da janela (`GEOMETRIC_CURRENT`) | vivo, somado em `:1047` | `m_floatingOffset` | sim | sim | `±1/−2` heurística | idem |
| `CBarPassElement::boundingBox` `BarPassElement.cpp:31` | via `assignedBoxGlobal` | vivo | **não** | sim | **não (já lógico ✔)** | **não** | snapshot por valor no elemento; somar `F` e a extensão |
| `damageEntire()` `:1135–1137` | via `assignedBoxGlobal` | vivo | não | `damageBox` converte | — | não | ver G3 |
| `cursorRelativeToBar()` `:1139–1141` | via `assignedBoxGlobal` | vivo | **não** | n/a (cursor global) | não | n/a | ver G2 |
| Tooltip `BAR = assignedBoxGlobal()` `:1304` | idem | vivo | **não** (a barra é desenhada com `F`) | sim | sim | n/a | snapshot do quadro, ver G2 |
| Preview `drawSnapPreview` `:514` | zona global | **n/a (screen-space)** | n/a | sim | sim, `round()` | n/a | sem `W`/`F` |
| `CPreviewPassElement::boundingBox` `PreviewPassElement.hpp:27–29` | — | — | — | — | devolve caixa **física** | — | dividir por `s` (contrato lógico) |

Achados: (1) o baseline já compõe `D+E+W+F−M` no desenho; a divergência é entre **desenho** (com `F`) e **bbox/damage/input/tooltip** (sem `F`, sem extensão inferior). (2) `W` e `F` do baseline vêm de estado vivo; o 0.57 os entrega congelados em `presentation`, e o bbox do baseline já é lógico (só o do preview é físico).

## 3. Decisões abertas de geometria

| ID | Questão | Opções | Pendência |
|---|---|---|---|
| **G1** | Semântica de `W`/`F` por modo de cena: `workspaceOffset` do `presentation` equivale ao `m_renderOffset` vivo no modo MONITOR e não-pinned? (H-GEO1) | Aceitar equivalência só em MONITOR; em cenas de workspace/replay o `presentation` manda (é o motivo de consumi-lo, B3 §2) | S6 (aritmética) + D15 |
| **G2** | Consumidores **sem** `presentation` (input, tooltip, `cursorRelativeToBar`, consultas fora do draw) | (a) cada `draw()` grava um `SPresentedGeometry{W,F,monitor,frameId}` por valor na barra, e os consumidores o usam; (b) recompor de estado vivo (diverge do desenho em animação) | Decisão do dono/arquiteto antes de 4.2. Recomendação provisória: (a), com carimbo de quadro/monitor e descarte se o quadro anterior |
| **G3** | `damageBox` não aplica `transformBoxForDamage` (ao contrário de `damageWindow`): `U:Renderer.cpp:3036–3048` vs `3071–3078` [EXT]. Saída rotacionada pode receber damage em espaço errado (H-GEO3) | Manter `damageBox` com união de caixa anterior e nova; avaliar `damageWindow` | D14 com `debug.damage_tracking` alternado (opção existe: `/usr/share/hypr/stubs/hl.meta.lua:87`) |
| **G4** | Ponto de enqueue do preview (screen-space) sem herdar o transformer da janela (H-PREV) | `RENDER_PRE_WINDOW` do dono vs `RENDER_POST_WINDOWS` vs de dentro de `draw()` | D12 e decisão com ensaio; preservar z-order observável (B3 §5) |
| **G5** | Política de captura/cena do tooltip (`m_renderingCapture`, `m_renderingSnapshot`, `workspaceScene`) | ocultar em captura vs manter | Decisão de comportamento; não filtrar sem aprovação |

## 4. Contratos executáveis sem compositor (insumo da primeira alteração)

Propriedades puramente aritméticas, testáveis fora do Hyprland (modelo `plugin/tooltip.hpp` + `plugin/test_tooltip.cpp` + `tools/test-plugin-tooltip.sh`, que o `check.sh` já executa).

**Precisões da revisão P2 (30/10/2026):** `D` é uma caixa; `E/W/F/M` transladam apenas sua posição. Definir `L={D.x+E.x+W.x+F.x−M.x, D.y+E.y+W.y+F.y−M.y, D.w, D.h}`: `F` entra **uma vez**, inclusive em P2. Extensão inferior vem antes de escala/round; AA é expansão lógica do bbox, não aumento obrigatório do desenho. `CBox::round` (Hyprutils instalado 0.14.3; fonte pública tag `v0.14.3`, `src/math/Box.cpp:51–62`) calcula `rx=std::round(x*s)` e `rw=std::round(x*s+w*s−rx)`, analogamente Y/H; não arredonda w/h independentemente. Empates seguem `std::round` (afastamento de zero).

Pré-condições do candidato (agora **verificadas** pelo módulo, ver contrato abaixo): entradas finitas, dimensões positivas, escala em `[2^-4, 2^4]`, extensão/AA finitos não negativos, espaço monitor-local comum para damage e aritmética IEEE usual, sem fast-math (`-O0/-O3` testados em GCC e Clang, sem fast-math). Não é validador de dados externos além desse domínio. `ceilOut` significa floor das origens e ceil das bordas direita/inferior; caixas representam intervalos semiabertos. Coverage aqui é **somente o modelo aritmético**, não AA efetivo, blur, rotação ou pixels finais do compositor. O valor de AA efetivo continua pendente de validação gráfica.

**Contrato numérico escolhido (P2.1, 30/10/2026) — resolve o bloqueio P3.** Em vez de construir limites conservadores com transformações livres de erro em toda soma/escala, o módulo **limita explicitamente o domínio** e devolve `std::nullopt` (nunca uma caixa errada) fora dele. Todas as funções retornam `std::optional`.

| Constante | Valor | Aplica-se a | Fundamento |
|---|---|---|---|
| `kMaxLogical` | 2^31 | \|componente\| de `D.x/y`, `E`, `W`, `F`, `M`, `extBottom`, `aaMargin`; `0 < D.w,D.h ≤ 2^31` | Posições/tamanhos Wayland (`wl_output`, `xdg_output`, `xdg_surface`) são int32 [EXT, protocolo; não lido nos fontes do host nesta sessão] |
| `kMinScale`, `kMaxScale` | 2^-4, 2^4 | escala | `CMonitor::m_scale` é `float` (`output/Monitor.hpp:79`); **não** foi encontrado limite fixo de escala de monitor nos headers 0.57.0 (o único intervalo "0.1..10" achado é de gesto); 1/16..16 é folga deliberada, não um limite do Hyprland |
| `kMaxPhysical` | 2^35 (= `kMaxLogical·kMaxScale`) | caixa física do preview | derivado |
| `kMaxBox` | 2^40 | entrada **e saída** de `unionDamage`; resultado de `logicalBBox`/`previewBBox` | `8·kMaxLogical ≤ kMaxBox` e `kMaxPhysical/kMinScale ≤ kMaxBox` (`static_assert`) |

**Por que um limite, e não só "finito".** (1) A perda de P3 é aritmética de ponto flutuante, não overflow: em `D={2^53,0,1,1}`, `s=1.5`, a soma `x+w` perde o `+1` (ulp de 2^53 é 2) e `ceilOut` fica com largura 0 enquanto `physicalLocal` dá 2. Acima de 2^53 a posição inteira em pixel nem é representável: não existe resposta correta a devolver. (2) Medição com a implementação **anterior** (200 000 amostras por magnitude, larguras 0,1–100, escalas 1/16..16): **0 falhas de cobertura até \|coord\|≈2^49; 1883 em 2^50; 7062 em 2^51; 12 195 em 2^52** (`evidence/P31-scan-orig-onset.log`; programa em `P31-scan.cpp.txt`). (3) A maior **borda** em pixels aceita é 2^45 (`2·kMaxBox·kMaxScale`; `static_assert ≤ 2^46`): 5 bits abaixo do início medido. (4) Um teto sozinho não basta contra perda de extensão em escala pequena: o guarda `resolved` (abaixo) é independente e tem mutantes próprios.

**Guarda `resolved(x,w) = x+w > x`.** Bordas são os doubles `x+w`/`y+h`, como o `CBox` do host. Extensão que não move a própria origem é degenerada e rejeitada: sem isso `physicalLocal({0.5,0,2^-54,1})` devolvia largura **−1** (revisão independente; reproduzido) e a união dava largura 0. Prova: se `x+ws > x` em double, `t=fl(x+ws) ≥ nextafter(x) > x` e `t−rx > x−rx ≥ −0,5`, logo `round(t−rx) ≥ 0`. Garantia vinculada à ordem de operações (soma lógica, depois escala), como no host; uma lasca de <1 ulp além da borda double não é observável por `CBox` e fica fora de contrato.

**Invariante de saída.** Toda `LogicalBox` devolvida por `logicalBBox`, `unionDamage` e `previewBBox` é entrada válida de `unionDamage` (`detail::boxOk`); se a união ultrapassar `kMaxBox` (ex.: duas caixas de largura `kMaxBox` em lados opostos) ou o preview colapsar a extensão (divisão/underflow), o resultado é `nullopt` na própria chamada.

**Decisão do dono (30/10/2026):** a Mudança 1 foi aprovada como contrato geométrico independente do Hyprland; H-DOM foi aceita **provisoriamente** com os limites acima, e o uso de `std::optional` nas quatro funções foi aprovado. Esses limites são **contrato interno sujeito a validação na integração**, não propriedades comprovadas do Hyprland. Tratamento de `nullopt` pelos consumidores: [NULLOPT-MATRIX.md](NULLOPT-MATRIX.md).

**O contraexemplo original permanece como teste de regressão:** `0x1p53` é **rejeitado** por `physicalLocal`, `logicalBBox`, `unionDamage` e `previewBBox`; o análogo em `kMaxLogical` é aceito e coberto. Fora do domínio: o chamador decide (descartar o quadro, não desenhar); o módulo nunca devolve resultado silenciosamente errado. **Não houve** inspeção dos fontes do Hyprland que prove que valores acima de 2^31 nunca ocorrem; o domínio é uma decisão técnica com folga (hipótese H-DOM, README §3), aceita provisoriamente pelo dono.

Alternativas avaliadas: (A) limites conservadores por TwoSum/`nextafter` em toda soma e escala — corrigiria o caso 2^53 mas é mais código, sem prova universal, e acima de 2^53 a resposta continua sem significado; (B) só rejeitar não finitos — insuficiente (o contraexemplo é finito); (C) **domínio explícito + `resolved` + invariante de saída** — escolhida: menos código, cada limite justificado e com mutantes próprios.

Em P4/P5, o candidato adiciona alargamento dirigido mínimo com `nextafter` quando o arredondamento comum perderia cobertura; as regressões independentes foram reproduzidas e corrigidas. Igualdade aproximada dos componentes não substitui a assertiva estrita de cobertura.

| ID | Propriedade | Variante que **deve** falhar (S7) |
|---|---|---|
| P1 | `physicalLocal(D,E,W,F,M,s) = round((D+E+W+F−M)·s)` para `s ∈ {1, 1.25, 1.5, 2}`, `M` com componentes negativas e `M ≠ 0` | `W` ou `F` somados duas vezes; `M` omitido; escala aplicada duas vezes |
| P2 | `logicalBBox` = a mesma caixa **sem** `×s`, **mais** `F`, **mais** extensão inferior `ROUNDING·3`, expandida pela margem de AA | bbox sem `F`; bbox em unidade física |
| P3 | Cobertura: `ceilOut(logicalBBox·s) ⊇ physicalLocal` para todo `s` do domínio (nenhum pixel desenhado fora do bbox declarado); fora do domínio: `nullopt` | `expand(10)` fixo sem extensão inferior reprova com `ROUNDING` grande |
| P4 | Damage = união(caixa anterior, caixa nova) cobre o desenho dos dois quadros | damage só da caixa nova |
| P5 | Preview: `logicalBBox = physicalBox / s` e ida-e-volta com `s` fracionário não cria lacuna de 1 px | devolver a caixa física como lógica |

Não cobrem: rotação de saída, `renderModif`, transformers, projeção (host), hit-test do host. Ver `RISKS-TESTS.md` para o que fecha cada um.

## 5. Regras de fronteira (resumo, o que o plugin nunca faz)

Nunca somar a posição da janela depois de `getEdgeDefinedPoint`; nunca reaplicar `alpha/fadeAlpha/workspaceAlpha` a `a` (já composto, `U:Renderer.cpp:657–682`); nunca subtrair origem de canvas nem pré-rotacionar (o host faz em `ElementRenderer.cpp:693–755`); nunca reutilizar um `CTexPassElement` após `clipRegion` transformada; nunca guardar `ctx`/`presentation` emprestados além da chamada (o elemento guarda **cópia** de `SWindowRenderPresentation`). Lista completa de erros: B3 §6.
