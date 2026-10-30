# Preparação da portabilidade Hyprland 0.57 (Marco P1)

Artefatos **preparatórios** das tarefas 1.2–1.4 da minuta externa `port-kwinish-bars-to-hyprland-0-57`. Não são especificação nem OpenSpec. A preparação P1 não executou build/testes (30/10/2026). **P2 executou build isolado e testes host-free**, com revisão independente via OpenAI Codex pelo Hermes; resultados e pendências em [RISKS-TESTS.md §6](RISKS-TESTS.md#6-resultados-p2--30102026). O candidato geométrico teve o bloqueio P3 resolvido em P2.1 ([RISKS-TESTS.md §7](RISKS-TESTS.md#7-resultados-p21--correção-isolada-do-bloqueio-p3-30102026)) e foi **aprovado pelo dono como contrato interno** (§8 do mesmo arquivo); a matriz de `nullopt` está em [NULLOPT-MATRIX.md](NULLOPT-MATRIX.md). Nenhuma compatibilidade funcional com o 0.57 é declarada; nenhum plugin foi carregado.

Base: branch `port-kwinish-bars-to-hyprland-0-57`, commit `9850593` (documental; difere de `b6d02db` só por `AGENTS.md` e `docs/DEVELOPMENT_STATE.md`). A minuta externa foi corrigida de `b6d02db` para `9850593`; diff em `/home/lhen/Projects/kwinish-plan-hyprland-0.57/CORRECAO-COMMIT-BASE.diff`.

## 1. Índice

| Arquivo | Tarefa | Conteúdo |
|---|---|---|
| [LEDGER.md](LEDGER.md) | 1.2 | O que o host retira no unload; 19 recursos com dono, invalidação, liberação, rollback; pontos de throw do init |
| [QUIESCENCE.md](QUIESCENCE.md) | 1.3 | Fatos sobre contexto único e unload; Q1–Q5 com observável, verificação, critério, controle negativo e limite; pontos de entrada |
| [GL-STATE.md](GL-STATE.md) | 1.4 | Estado × conteúdo do stencil; convenções do upstream; escada E0/E1/E2; saídas de `renderPass` |
| [GEOMETRY.md](GEOMETRY.md) | 1.4 | Mapa baseline→port por consumidor; decisões G1–G5; contratos aritméticos P1–P5 |
| [NULLOPT-MATRIX.md](NULLOPT-MATRIX.md) | 4.2/4.4 (prep.) | Tratamento de `std::nullopt` por consumidor (desenho, damage, input, preview, tooltip); proposta, não implementada |
| [RISKS-TESTS.md](RISKS-TESTS.md) | 1.2–1.4 | B1–B3 × testes × aceite; estratégia B2/B3; proposta da primeira alteração C++ |
| [P5-WINDOW-API.md](P5-WINDOW-API.md) | 3.1 (parcial) | P5: decisões G2/input/snap/damage/`nullopt`; matriz de equivalência da janela 0.56.2→0.57; sondas; diff de include/acessores; comparação de erros; revisão |
| [P6-LOG-API.md](P6-LOG-API.md) | 3.1 (parcial) | P6: Log API 0.56.2→0.57 (5 chamadas); sondas de tipo e de execução contra o header 0.56.2; diferença medida de alocação; comparação de erros; revisão (reprovada sob leitura estrita); **aprovada pelo dono** com a diferença de alocação aceita; inventário inicial de `g_pKeybindManager` |
| [P7-KEYBINDS.md](P7-KEYBINDS.md) | prep. (sem código) | P7: matriz keybinds/mouse-bind/dispatchers 0.56.2→0.57 (K1–K13, classes BC/RD/DI); `changeMouseBindMode`/`dragBegin`/`dragEnd`/`exclusiveDeviceGrab`; proposta Q4 (posse por sessão via `ended`); sonda K; revisão (aprovada) |
| [P8-DISPATCHERS.md](P8-DISPATCHERS.md) | 3.1 (parcial) | P8-A: dispatchers K4–K8/K10–K13 → `Config::Actions`/`executor()` (janela focada, guardas do tradutor, `ActionResult` com log limitado); equivalência com host falso + tradutor 0.56.2 verbatim; 45 mutações; sonda A; comparação de erros; revisão (aprovada, 4 rodadas); RD de K13 aceito pelo dono |
| [P8-DRAG.md](P8-DRAG.md) | 3.1 (parcial) | P8-B: início de drag K1/K2 → `KwDrag::begin` (corpo 0.56.2 sobre as primitivas 0.57; não encerra nem substitui drag ativo); **K3, K9 e rearm bloqueados** (sem posse por sessão = Q4), com demonstração; host falso + corpo verbatim, oráculo exaustivo, 18 mutações, sonda B; revisão (aprovada); aprovada pelo dono e commitada (parcial) |

Relatórios longos permanecem em `/home/lhen/Projects/kwinish-plan-hyprland-0.57/investigacao-b1-b3/` e não foram duplicados.

## 2. Legenda de evidência

| Marca | Significado |
|---|---|
| `K:` ou `arquivo:linha` sem prefixo | plugin pré-port desta worktree |
| `H:` | header instalado `/usr/include/hyprland/src/…` (Hyprland 0.57.0 `0aa6745f`) |
| `U:` | fonte upstream fixada em `…/investigacao-b1-b3/evidence/upstream/Hyprland/src/` (mesmo commit) |
| `[EXT]` | herdado dos relatórios B1–B3, não reconferido nesta rodada |
| `[N]` | **verificado nesta rodada** e ausente ou menos preciso nos relatórios |
| Hxx | hipótese **não demonstrada** |

## 3. Registro de hipóteses (não são fatos)

| ID | Hipótese | Onde | Fecha com |
|---|---|---|---|
| H-DTOR | Destrutores estáticos do módulo rodam no `dlclose` | LEDGER | S5 + D1 |
| H-DEL | Deleters instanciados no módulo não rodam após o `dlclose` | LEDGER | S5 + D1 |
| H-REG | Caminho `false` de `addWindowDecoration`: propriedade permanece no SP nomeado do chamador; temporário morre ao fim da expressão; hipótese anterior de destruição imediata foi retificada em P2 | LEDGER | task 3.3: rollback/retentores |
| H-THREAD | Render, EventBus e IPC numa thread; unload IPC não aninhado | QUIESCENCE | D1/D6 |
| H-Q3 | Sem emissão com snapshot nosso no unload (IPC ou `reload`) | QUIESCENCE | argumento + B1-NEG |
| H-CNT | Guardas RAII não alteram a ordem de destruição | QUIESCENCE | revisão |
| H-GL1 | Máscara de stencil herdada `0x00` neutraliza o recorte do baseline | GL-STATE | D9 |
| H-GL2 | O "cleanup" `glClear` do baseline limpa o attachment inteiro (scissor já desligado) | GL-STATE | D9 |
| H-GL3 | Conteúdo de stencil é scratch (nenhum consumidor upstream o herda) | GL-STATE | D9 |
| H-GL4 | Textura vinculada por `createTexture/renderText` não afeta o draw seguinte | GL-STATE | D10 |
| H-GL5 | Sem `workspacePresentation`/`blurOwner` o blur da barra é planejado com apresentação default | GL-STATE | D11/D15 |
| H-GEO1 | `presentation.workspaceOffset/floatingOffset` substituem os valores vivos do baseline em MONITOR não-pinned | GEOMETRY | S6 + D15 |
| H-GEO3 | `damageBox` sem `transformBoxForDamage` erra damage em saída rotacionada | GEOMETRY | D14 |
| H-PREV | Ponto de enqueue do preview preserva z-order | GEOMETRY | D12 |
| H-DOM | Nenhuma posição/tamanho lógico do compositor excede 2^31 em módulo e a escala fica em [1/16, 16] (domínio de `geometry.hpp`; fundamento: int32 do Wayland, `m_scale` float; **não** verificado nos fontes do Hyprland) | GEOMETRY §4 | dono aceita o domínio; D12/D13 em escalas reais |

## 4. Resumo do que mudou em relação aos relatórios B1–B3

1. **Q1 simplificado, condicional:** `reset()` limpa `m_pass`; fora de sessão, sem enqueue/retenção externa e sob H-THREAD, não há passe nosso vivo. `transformedPass` local permanece coberto pela sessão ativa antes de ser movido. Registry/`removeAllOfType` não são necessários nesse domínio; `renderer.m_renderPass` e `m_renderData` não existem no 0.57.
2. **Segundo caminho de unload** (config-managed, entre `preReload` e `reloaded`) e **ordem obrigatória**: `~CHyprBar` usa `g_pGlobalState`, destruído pelo host depois de `PLUGIN_EXIT`.
3. **Armadilha upstream:** `m_registeredDecorations` é append-only; unload compara endereços liberados.
4. **Stencil:** convenção scratch compartilhada por monitor; o "cleanup" do baseline roda sem scissor.
5. **Geometria:** o desenho já compõe `D+E+W+F−M`; bbox, damage, input e tooltip omitem `F`; só o bbox do preview está em unidade errada.
6. **Contagens verificadas no baseline:** 10 listeners globais e 29 valores de config.
