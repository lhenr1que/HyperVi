# Riscos B1–B3 × testes × critérios de aceite; primeira alteração C++

Preparatório P1; resultados executados em P2 estão na §6. Associa cada risco aos testes e critérios da minuta (`design.md` 9.6/9.7, `tasks.md`) e aos IDs S/D de `investigacao-b1-b3/TESTES-E-GATES.md`. As seções 1–5 preservam a proposta anterior, com retificações P2 abaixo. Estados: **estático** (leitura/grep), **simulado** (host-free), **GL real** (compositor), **aninhado** (instância Hyprland descartável).

## 1. B1 — lifecycle (quiescência)

| Teste | Tipo | Aceite | Insumo desta rodada | Fecha |
|---|---|---|---|---|
| B1-S1 (S2) ledger | estático | Todo `listen/addLuaFunction/createFunctionHook/setUpdateCallback/CEventLoopTimer/add*` tem linha no ledger, rollback e teardown; aquisição sem linha reprova | `LEDGER.md` L01–L19; entradas em `QUIESCENCE.md` §3 | Estrutura; não prova runtime |
| B1-S2 (S3, D5) init parcial | simulado + aninhado | Exceção injetada após cada aquisição de §3 do ledger desfaz **só** o adquirido; `shutdown()` repetido não duplica; efeitos L03 desregistrados | Pontos de throw enumerados (ledger §3) | Rollback de init |
| B1-S3 (S5) deleters | estático no `.so` | `readelf -d` sem `NODELETE`; `nm` sem símbolos `UNIQUE`; origem dos deleters/vtables dos passes e de `UP/SP` listada | H-DTOR, H-DEL | Pré-requisito de D1 |
| Q5 grep | estático | 0 ocorrências do comando de `QUIESCENCE.md` Q5 | Rodado em 30/10: 0 no baseline | Q5 (baseline) |
| B1-D1 | aninhado | Load/unload/reload entre quadros: `active()==false`, contadores Q1=0 e Q2=0 ao `dlclose`; `config.reloaded` emitido após unload não chama o módulo | Q1, Q2; passo 7 do host | Q1, Q2, H-DTOR |
| B1-D2/D3 | aninhado | Tooltip armado/visível, drag, preview, hook ativos: após o passo 2 estado limpo e modo de mouse-bind restaurado; nenhuma chamada ao módulo depois do passo 8 | Q4, L17 | Q4 |
| B1-D4 | aninhado | Janela destruída entre enqueue e draw: omite desenho; contador de passes volta a 0 | L14 | Q1 |
| B1-D6/D7 | aninhado | Fronteira quiescente para signal/timer/idle/animação como **premissa do harness**; closure Lua antiga devolve erro por ID | Q3 (não observável), L05 | Limite listado |
| B1-NEG | aninhado, opcional | Sonda que chama `plugin unload` de dentro de listener: resultado **descritivo** | Q3 | Sem afirmação de suporte |

**Estado B1 após esta rodada:** condicionado, **não resolvido**. Ganho: Q1 passa de "contador de passes" a "`active()==false` + fato de contexto único" para o contexto padrão; `removeAllOfType` e registry de filas deixam de ser exigidos; ordem do protocolo ganha a restrição L02 (não resetar `g_pGlobalState` antes das barras). Permanecem abertos: H-DTOR, H-DEL, H-THREAD, H-Q3.

## 2. B2 — estado gráfico e composição

| Teste | Tipo | Aceite | Insumo | Fecha |
|---|---|---|---|---|
| B2-S1 (S9, S10) | estático/simulado | Guards cobrem só o que declaram; em E0, `grep -nE 'glStencil\|glClear\|glColorMask\|glScissor\|glEnable\|glDisable'` em `plugin/*.cpp` ⇒ 0; matte com R = cobertura e UV coextensivo; variante que altera só A falha | `GL-STATE.md` §3 | Escolha E0 vs E1 (parte estática) |
| B2-D9 | aninhado, GL real | Padrão de stencil conhecido preservado (E0) **ou** documentado como scratch sem consumidor dependente (E1), inclusive região **fora** da caixa da barra e com máscara herdada `0x00`; variante que limpa o attachment inteiro reprova | H-GL1, H-GL2, H-GL3 | Decide E0/E1 |
| B2-D10 | aninhado, GL real | Scissor, viewport, blend, color mask, bindings e caches coincidem após cada saída (incl. retornos antecipados, `GL-STATE.md` §4) e o draw seguinte não muda | H-GL4 | Estado/caches |
| B2-D11 | aninhado, GL real | Blur off/on (precomputed/live/shader), sem vazamento nos cantos; `a`/fade uma vez; `workspacePresentation` e `blurOwner` repassados; troca animada de workspace sem erro de blur | H-GL5 | Composição |

**Estratégia de validação aplicável a B2 (nenhuma etapa executada):**
1. *Estática, já possível:* grep S9 e leitura de cada saída de `renderPass`.
2. *Sonda de estado, a construir depois, fora de produção:* um plugin de teste separado (scratch, nunca no repo de produção) que, em `render.stage` com contexto — `RENDER_PRE_WINDOW` (`U:Renderer.cpp:680`, emitido antes das decorações e do `redirectPass` em :713) e `RENDER_POST_WINDOW` (:898) — enfileira dois passes custom: o primeiro grava um padrão de stencil conhecido e parâmetros não-default e registra `glGet*`; o segundo lê o stencil por `glReadPixels(GL_STENCIL_INDEX)` e os parâmetros, e escreve um JSON. Como a ordem de desenho é a ordem de enqueue, bracketing da barra de uma janela sem transformers é determinístico. A sonda faz GL cru de propósito; o plugin sob teste não.
3. *Ambiente:* instância Hyprland **descartável** (HOME/XDG temporários, assinatura própria). Disponíveis no host, só consultados por `which`/`pkg-config` (não executados): Hyprland 0.57.0, `grim`, `g++`/`clang++`, EGL mesa. A seleção de backend headless/aninhado (p.ex. `AQ_BACKENDS`) **não foi confirmada**; confirmar na documentação do Aquamarine antes de escrever o roteiro L3.
4. *Critério de parada:* se o ambiente não isolar (sem GL real descartável), o gate B2 permanece aberto (design 5); mock não substitui.

## 3. B3 — geometria e overlays

| Teste | Tipo | Aceite | Insumo | Fecha |
|---|---|---|---|---|
| B3-S1 (S6, S7, S8, S11) | simulado host-free | P1–P5 de `GEOMETRY.md` §4, com variantes errôneas que **falham**; rotação de região documentada como no-op conhecido (`U:OpenGL.cpp:2644–2657`) | **Primeira alteração C++ (§4)** | Aritmética |
| B3-D12/D13 | aninhado | Barra+preview+tooltip no mesmo quadro; escalas 1, 2, 1.25, 1.5; origens negativas; z-order, bbox e hit regions coerentes | G4, G2 | Overlays |
| B3-D14/D15 | aninhado | Rotação 0/90/180/270; pinned, troca animada de workspace e `floatingOffset`: damage parcial idêntico ao redraw completo | G1, G3, H-GEO1, H-GEO3 | Clip/damage |
| B3-D16/D17 | aninhado | Captura/offscreen/nested; input/touch/botões sem transformação reaplicada | G5 | Entrada |

**Estratégia de validação aplicável a B3:**
1. *Host-free primeiro* (`GEOMETRY.md` §4): fecha a aritmética sem compositor.
2. *Equivalência de damage por diferença de pixels (aninhado):* renderizar a mesma cena estática duas vezes, com `debug.damage_tracking` alternado (a opção existe nos stubs do Hyprland instalado, `/usr/share/hypr/stubs/hl.meta.lua:87`), capturar com `grim` e exigir imagens idênticas por escala e rotação. Cobre clip e damage sem instrumentar o plugin.
3. *Z-order do preview (G4):* comparar capturas com o preview enfileirado em cada candidato (PRE_WINDOW / POST_WINDOWS / dentro de `draw`) contra a baseline 0.56 documentada; decisão por evidência visual, não por leitura.
4. *Não confirmado:* geração de arrastar/ponteiro sintético na instância descartável (`hyprctl dispatch movecursor`, `hl.plugin.kwinish.grab`) — verificar no roteiro L3; sem isso D12 fica parcial.

## 4. Proposta objetiva da primeira alteração C++

**Mudança 1 — contrato geométrico host-free.** Escolhida por três motivos: não depende do build 0.57 (hoje vermelho), codifica B3 em teste executável (P1–P5) e não toca nenhum arquivo de produção existente.

| Item | Conteúdo |
|---|---|
| Arquivos novos | `plugin/geometry.hpp` (só `<cmath>`/`<algorithm>`, structs próprias como `tooltip.hpp`, sem headers do Hyprland); `plugin/test_geometry.cpp`; `tools/test-plugin-geometry.sh` (cópia do runner do tooltip: `g++ -std=c++2b -Wall -Wextra -Werror`). `tools/check.sh` já executa todo `tools/test-*.sh` |
| Arquivos existentes | **Nenhum alterado.** `plugin/Makefile` (`SRC`) intacto; o header só é incluído pela produção em mudanças posteriores (4.2/4.4) |
| Vermelho primeiro (AGENTS) | 1) escrever o teste e o runner; 2) `bash tools/test-plugin-geometry.sh` ⇒ `FALHOU - compilar` (header ausente) = vermelho registrado; 3) criar `geometry.hpp`; 4) verde. Sensibilidade (S7): cada variante errônea é uma função do teste que **precisa** diferir do resultado correto em ao menos uma entrada |
| Conteúdo mínimo | `physicalLocal(D,E,W,F,M,s)`, `logicalBBox(…, extBottom, aaMargin)`, `unionDamage(prev, next)`, `previewBBox(physical, s)`; escalas `{1,1.25,1.5,2}`, `M` negativo, `W`/`F` ≠ 0 |
| Validação autorizada necessária | compilar e executar **apenas** o teste host-free (g++, sem Hyprland, sem GL); opcional `bash tools/check.sh` para registrar que o passo `plugin` continua vermelho pelo mesmo motivo de antes (comparar com o log de 2.2) |
| Aceite | teste vermelho→verde com saída real colada em `DEVELOPMENT_STATE.md`; `git status --porcelain` = 3 arquivos novos e nenhum modificado; variantes S7 falham como esperado |
| Fora do escopo | includes/API do 0.57, `BarPassElement`, GL, ownership, lifecycle |

**Ordem recomendada:** (a) com autorização de build isolado, tarefa 2.2 (reproduzir o vermelho do plugin com `TARGET` temporário, sem alterar código); (b) Mudança 1; (c) tarefa 3.1 por família de header, cada uma com TU de sonda e vermelho próprio. A (a) não é pré-requisito técnico da (b), mas dá a referência para saber se `check.sh` piorou.

## 5. Decisões pendentes (dono) e hipóteses abertas

Decisões: **G2** (snapshot para consumidores sem `presentation`), **G4** (ponto de enqueue do preview), **G5** (política de captura do tooltip), **E0 vs E1** (decidida por D9/D11, não por opinião), exposição dos contadores Q1/Q2 ao harness. Build 2.2 e Mudança 1 foram autorizados e tentados em P2; isso não autoriza continuação futura. Hipóteses: ver [README](README.md) §3.

## 6. Resultados P2 — 30/10/2026

> **Atualização P2.1:** o bloqueio P3 descrito nesta seção foi tratado na §7 (contrato numérico com domínio explícito). Esta seção preserva o histórico de P2 sem reescrevê-lo; onde diz "P3 reprovado" ou "suíte vermelha", vale a §7.

### Revisões independentes e limites

Claude, Codex CLI e Gemini CLI não encontrados; sem instalação. Alternativa efetivamente executada: sessões novas via `hermes chat --safe-mode --provider openai-codex -m gpt-6.1-sol -t web --max-turns 1 --reasoning high -Q --query-file <prompt>`, somente material fornecido, sem ferramentas de escrita. Os relatórios de código são JSON; o de contratos é parecer textual. Uma primeira tentativa com `--run-budget` foi recusada pela CLI gateway (exit 2); removido apenas o argumento, sem alterar configuração global.

**Desvio de sequência:** revisão externa dos contratos obtida após a primeira implementação; recebeu apenas os seis snapshots P1 anteriores e trechos de fontes, não o candidato. Nenhum código foi editado enquanto as revisões estavam em curso. Isso não equivale a cumprir a ordem originalmente pedida para a Etapa 1.

| Classificação | Parecer de contratos |
|---|---|
| Confirmado | Ordem `PLUGIN_EXIT` → remoção de decorações → `dlclose`; destrutor da barra usa global. `D+E+W+F−M` no desenho baseline. Ambiguidade de round: `CBox::round` arredonda origem e extensão residual, não tamanho independente. `SP&&` não transfere propriedade nos retornos `false` mostrados. |
| Condicionado | Q1 depende de sessão ativa durante a propriedade local de `transformedPass`, sem enqueue/retenção externa e sem concorrência/reentrância. Q3 inclui snapshots/SP de decoração, timers/idles/animação; origem administrativa não basta. Equivalência de presentation, valor de AA, conteúdo scratch e estado/caches GL exigem provas adicionais. |
| Bloqueado | Declarar lifecycle/unload seguro, escolher E0/E1 por leitura, afirmar preservação de stencil ou liberar integração de produção. P3 do candidato permanece reprovado. |

Retificações estão em `QUIESCENCE.md`, `LEDGER.md`, `GL-STATE.md` e `GEOMETRY.md`. O parecer independente completo é [`evidence/P2-review-contract.txt`](evidence/P2-review-contract.txt).

### Baseline de compilação isolado

```sh
make -C /home/lhen/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/home/lhen/.hermes/cache/scratch/kwinish-p2/build/kwinish-bars-baseline.so
```

HOME/XDG temporários sob o mesmo scratch, sem DISPLAY/WAYLAND_DISPLAY/HYPRLAND_INSTANCE_SIGNATURE. Hyprland de `pkg-config`: **0.57.0**. `make` **exit 2**; nenhum alvo `.so` criado. Erros reais: `hyprland/src/desktop/view/Window.hpp` ausente em main/barDeco; métodos de passes não sobrescrevem mais a API; `m_renderData` removido; `renderRect` agora exige `CRenderContext&`. Comando de compilação expandido e erros integrais: [`evidence/P2-build.log`](evidence/P2-build.log); metadados/exit em `P2-build.json`. Isso reproduz a baseline vermelha; não é build do candidato integrado.

### Mudança 1 e ciclos executados

Somente os três arquivos de código autorizados foram criados; nenhum fonte ou Makefile preexistente alterado. `geometry.hpp` inclui apenas `<algorithm>`/`<cmath>`, usa structs próprias distinguindo caixas lógicas/físicas, sem Hyprland/GL. `physicalLocal`, `logicalBBox`, `unionDamage` e `previewBBox` não foram integrados à produção. Runner compila com `-std=c++2b -Wall -Wextra -Werror` em diretório temporário e remove somente esse diretório.

| Evidência | Resultado real |
|---|---|
| `P2-red-1.log` | exit 1: header ausente (`FALHOU - compilar`) |
| `P2-green-1.log` | exit 0: primeiro oráculo P1 |
| `P2-red-2.log` / `P2-green-2.log` | API P2 ausente → implementação → exit 0 |
| `P2-red-3.log` / `P2-green-3.log` | APIs P4/P5 ausentes → implementação → exit 0, 594 verificações |
| `P2-review-code.txt` / `P2-red-review-p5.log` | revisão 1 reprovada; regressão P5 reproduzida (exit 1), sem tolerância na cobertura |
| `P2-green-review-p5.log` | correção P5 → exit 0, 3031 verificações, incluindo sweep realmente negativo e previews fracionários |
| `P2-review-code-final.txt` / `P2-red-review-p4.log` | revisão 2 reprovada; regressão P4 reproduzida (exit 1) nos limites fracionários da união |
| `P2-green-final.log` / `P2-green-clang.log` | correção P4 → exit 0 em GCC/Clang, **3035 verificações**, 13 negativos rejeitados |
| `P2-review-code-verdict.txt` | revisão 3 **reprovada por P3**; P4/P5 sem outro defeito confirmado nessa revisão |
| `P2-red-final-p3.log` / `P2-red-final-p3-clang.log` | regressão P3 incorporada, **estado final exit 1** em GCC/Clang |

Os verdes intermediários não são o estado atual. O teste final foi deixado vermelho deliberadamente para não ocultar o achado: `D={0x1p53,0,1,1}`, offsets zero, escala `1.5`, extensão/AA zero. `logicalBBox` seguido de soma da borda antes de escala produz coverage físico com largura 0; `physicalLocal` produz largura 2. Todos os valores/intermediários são finitos. Limitar coordenadas sem decisão explícita alteraria o contrato; após dois ciclos de correção/revisão, o caso foi registrado e a implementação interrompida, **não aprovada**.

### Negativos: evidência de sensibilidade, não só casos positivos

No verde intermediário final, cada variante executou as mesmas assertivas da suíte e terminou **1** com mensagem `FALHOU - P...`; o runner rejeita exit 0, sinal, erro de CLI ou compilação como evidência negativa. Classes: W duplicado; F duplicado; M omitido; escala dupla; sinal de M errado; bbox sem F; bbox físico; `expand(10)` sem extensão (rejeitado especificamente por P3); damage só novo; preview físico tomado como lógico; round independente de w/h; largura inconsistente; dimensões de preview sem divisão por escala.

Além desses **13 modos**, foram feitas **13 mutações reais no header**, compiladas em cópias scratch com os mesmos flags; todas compilaram com **0** e a suíte positiva sem modo negativo as rejeitou com **1**. Logs e resultados estruturados: `evidence/P2-mutants-final.log`, `evidence/P2-mutants.json`. As mutações não foram aplicadas ao repositório. Como o bloqueio P3 agora falha na suíte positiva, os resultados anteriores não devem ser recontados a partir de uma suíte já vermelha: nesse estado uma falha inespecífica não prova sensibilidade à mutação.

### Estado e continuação

**Mudança 1 parcial/reprovada.** Não integrar nem seguir para headers/render/lifecycle. Próxima alteração recomendada: resolver explicitamente o contrato numérico/coverage de P3, manter a regressão, obter verde e nova revisão independente. Nenhum commit, merge, plugin carregado, teste gráfico, instalação, configuração global, OpenSpec ou outra worktree alterados. `tools/check.sh` não executado. O aceite antigo de §4 sobre status exclusivamente com três arquivos novos é retificado: as atualizações documentais autorizadas são exceção; fontes existentes continuam intactos. Próxima sessão exige nova autorização.

## 7. Resultados P2.1 — correção isolada do bloqueio P3 (30/10/2026)

Escopo: somente `plugin/geometry.hpp`, `plugin/test_geometry.cpp`, `tools/test-plugin-geometry.sh` e documentação. Contrato numérico escolhido e justificativa: [`GEOMETRY.md`](GEOMETRY.md) §4. Sem integração, plugin carregado, teste gráfico, instalação, commit, `tools/check.sh` ou outra worktree.

### Investigação (antes de alterar código)

Perda de extensão: em `D={2^53,0,1,1}`, `s=1.5`, `x+w` arredonda a 2^53 (ulp 2) e `ceilOut` tem largura 0; `physicalLocal` dá 2. Varredura com a implementação anterior: 0 falhas até 2^49, 1883/200 000 em 2^50, 12 195 em 2^52 (`P31-scan-orig-*.log`). O Hyprland 0.57.0 instalado guarda a escala como `float` e a posição como `Vector2D` (double); não foi achado limite fixo de escala de monitor (só um "0.1..10" de gesto). Domínio efetivo: int32 do protocolo Wayland (fonte externa, não lida nos fontes). Alternativas: (A) TwoSum/`nextafter` em toda soma — mais código, sem prova universal, e acima de 2^53 não há resposta correta; (B) só rejeitar não finitos — insuficiente; (C) domínio explícito + guarda de extensão resolvível + invariante de saída — **escolhida**.

### Ciclos executados

| Evidência | Resultado |
|---|---|
| `P31-red-1.log` | teste novo contra o header antigo: **exit 1**, não compila (API/constantes ausentes) |
| `P31-red-2.log` | header com `std::optional` mas guardas neutros: **exit 1**, `FALHOU - P3 original counterexample 2^53 rejected by physicalLocal` |
| `P31-green-gcc.log` / `P31-green-clang.log` | guardas reais: suíte **320 762 verificações, exit 0**; 13 negativos e **46 mutações reais** do header, cada uma rejeitada pela assertiva **esperada** (runner compara a mensagem, não só o exit) |
| `P31-sanitize.log` | UBSan (`undefined,float-divide-by-zero,float-cast-overflow`, sem recuperação) em GCC e Clang: exit 0 |
| `P31-opt-levels.log` | `-O0` e `-O3` em GCC e Clang: exit 0 (sem fast-math) |
| `P31-exact-longdouble.log` | 3 000 000 amostras no domínio com oráculo em `long double` (x87): 0 casos de `physicalLocal` fora do `ceilOut` exato e 0 fora do `ceilOut` do bbox em double (oráculo e programa: `P31-exact.cpp.txt`) |

Testes de fronteira: cada um dos 12 campos lógicos e da escala é testado em `+/-limite` (aceito), `nextafter` além do limite, ±inf, NaN, `DBL_MAX`, `2^53` (rejeitado); comprimento/escala zero, negativo, abaixo do mínimo; `extBottom`/`aaMargin` fora de `[0,2^31]`; 32 combinações de sinal nos cantos do domínio × 11 escalas (inclui `nextafter` das bordas) × extensões/AA no limite; fuzz determinístico de 60 000 casos concentrado nos limites (P3, P4 união, P5 preview, reaceitação da saída). Resultados não finitos/overflow: rejeitados por entrada (`within`/`length`/`scaleOk` são falsos para NaN; overflow de `w·scale` é pego por `resolved`).

Mutações: além das 13 de P2 e das de domínio por guarda (limite aberto, sem `fabs`, sem teto de comprimento/escala/ext/AA, escala sem mínimo/máximo, um eixo só, operando único da união, constantes aumentadas → `static_assert`), foram acrescentadas mutações do guarda `resolved`, dos pós-testes de saída e dos dois alargamentos `nextafter` de `previewBBox`. Estas duas últimas **sobreviveram** à suíte de P2 (lacuna real) e geraram o teste "ulp-adjacent". Várias expectativas de mensagem foram corrigidas depois de observar que outra assertiva legítima e mais específica detectava antes; nenhuma foi aceita sem essa verificação. Uma mutação (`v>0` → `v>=0`) tornou-se equivalente em `physicalLocal` (redundante com `resolved`) e passou a ser distinguida por um teste de `logicalBBox` com AA/extensão.

### Revisão independente

Revisor: `gpt-6.1-sol` (provedor `openai-codex`), sessões novas via `hermes chat --safe-mode`, somente material fornecido, sem ferramentas — **família diferente** do implementador (Claude). Claude/Codex/Gemini CLI não existem no PATH; nada foi instalado. O revisor não executou código: os resultados reais acima são do implementador.

| Rodada | Parecer | Tratamento |
|---|---|---|
| 1 (`P31-review-code.txt`) | reprovado: 3 major (largura física −1 em `{0.5,0,2^-54,1}`; união com largura 0; preview com borda não representável) + 1 minor (oráculo repete a soma) | 1º e 2º reproduzidos e corrigidos com `resolved`; 3º/oráculo: decisão explícita "borda = double `x+w`, ordem de operações especificada" + oráculo `long double` independente. Revisor não achou contraexemplo direto de P3 |
| 2 (`-2.txt`) | reprovado: 2 major (união de caixas válidas gera caixa que a própria função rejeita; preview pode colapsar extensão) | invariante de saída por construção (`boxOk` nas três saídas), testes e mutantes novos |
| 3 (`-3.txt`) | **aprovado**; sem blocker/major; 1 minor (a justificativa deve falar de **bordas** 2^45, 5 bits e não 6 abaixo de 2^50) | comentário e `static_assert` corrigidos (`2·kMaxBox·kMaxScale ≤ 2^46`); reexecutado tudo após a correção |

Limitações do revisor (declaradas por ele): amostragem de larguras sub-ulp/subnormais é por casos dirigidos; a exclusão `draw.h > kMaxLogical` em `covered` não testa cobertura dessas extensões (a rejeição por campo é testada). A aprovação é de revisão estática, não prova universal.

### Estado

**Mudança 1: P1–P5 verdes em GCC e Clang; bloqueio P3 resolvido por contrato de domínio justificado; negativos individualmente conclusivos. Recomendável para aprovação pelo dono**, com estas condições abertas: (a) o dono aceita o domínio (H-DOM: 2^31, escala 1/16..16); (b) a API passou a `std::optional` — a integração futura deve tratar `nullopt` explicitamente; (c) coverage continua sendo **só o modelo aritmético**, sem AA efetivo, blur, rotação ou pixels do compositor. Não aprovado: integração, headers 0.57, B1/B2/B3 (continuam condicionados).

## 8. Aprovação e fechamento da Mudança 1 (30/10/2026)

**Decisões do dono:** Mudança 1 aprovada como contrato geométrico independente do Hyprland; H-DOM aceita provisoriamente (componentes lógicos ≤ 2^31, escala [1/16, 16], preview físico ≤ 2^35, caixas derivadas ≤ 2^40, guarda de extensão representável, rejeição explícita de entradas e saídas fora do contrato); `std::optional` nas quatro funções aprovado. **Não** são propriedades comprovadas do Hyprland: são contrato interno a validar na integração.

**Conferência código × contrato:** `kMaxLogical=0x1p31`, `kMaxPhysical=0x1p35`, `kMaxBox=0x1p40`, `kMinScale=0x1p-4`, `kMaxScale=0x1p4`, `detail::resolved`, `detail::boxOk` nas três saídas e `std::optional` em `physicalLocal/logicalBBox/unionDamage/previewBBox`; `static_assert`s do teste fixam as relações. Hashes dos três arquivos inalterados desde P2.1 (`dccfe88e…`, `e3849259…`, `51bf2c4a…`).

**Execução:** suíte geométrica GCC e Clang exit 0 (320 762 verificações, 13 negativos, 46 mutações). `bash tools/check.sh` (HOME/XDG temporários, plugin compilado em temporário): Lua 11 arquivos OK; tools 5 arquivos OK (inclui `test-plugin-geometry.sh`); Rust 31 testes OK; **único** passo vermelho: `make -C plugin`, com os **mesmos** 6 erros únicos de `evidence/P2-build.log` (diff idêntico) — falha conhecida contra Hyprland 0.57.0, não regressão. Log filtrado: `evidence/P4-check.log`. Efeito colateral: o `cargo test --locked` baixou crates para `~/.cargo` (cache do cargo; nada instalado no sistema).

