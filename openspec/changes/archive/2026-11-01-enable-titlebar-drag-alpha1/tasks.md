# Tasks

Estado em 01/11/2026: proposta **aprovada pelo dono**; implementação feita no working tree (ver `docs/DEVELOPMENT_STATE.md`), sem commit e sem teste ao vivo. O `floatWindow` do Lua já foi decidido (débito, item 1.2). A implementação depende de autorização própria dos arquivos. Critérios em design.md, Validation Plan.

## 1. Pré-requisitos

- [x] 1.1 Arquivar `disable-titlebar-drag-alpha0` antes desta mudança (os deltas daqui removem e renomeiam requisitos dela); conferir que suas tarefas abertas estão resolvidas ou registradas.
- [x] 1.2 Registrar a decisão do dono sobre o `floatWindow` do núcleo Lua: **débito** (01/11/2026, design.md Open Questions 1); o núcleo não muda.
- [x] 1.3 Confirmar base Git e arquivos autorizados: `plugin/barDeco.{cpp,hpp}`, `plugin/main.cpp`, `plugin/globals.hpp`, `plugin/barInput.hpp`, `ui/src/menu.rs`, testes, `docs/API.md`, `docs/CONFIGURAVEL.md`, `docs/STATUS.md`, `docs/DEVELOPMENT_STATE.md`.

## 2. Vermelho primeiro (host-free)

- [x] 2.1 Inventário estrutural: nenhuma rota compilada pede fim de drag (`MBIND_INVALID`, `endDragTarget`, `0movewindow`); rotas de toque continuam ausentes.
- [x] 2.2 Decisões de press/soltura em `barInput.hpp` com resultado de drag: início só com `BEGIN_STARTED`, barra cede a drag ativo (esquerdo e direito), soltura do drag não tomada, clique simples segue o par.
- [x] 2.3 Grab do menu: armado só com `BEGIN_STARTED`, limpo em tecla, clique sem drag ativo repassado.
- [x] 2.4 Rearm por Shift: só início, só com a barra em drag, sem unsnap pendente, ponteiro sobre a janela da barra.
- [x] 2.5 Guarda do `floatWindow` C++ com drag ativo.
- [x] 2.6 Teste Rust do menu: Mover/Redimensionar presentes (o teste atual da alpha-0 passa a falhar de propósito; registrar a troca de oráculo).
- [x] 2.7 Demonstrar vermelho de cada item no HEAD da alpha-0 por assertiva, não por erro de compilação.

## 3. Implementação

- [x] 3.1 Religar K1, unsnap antes do início, encaixe ao soltar, Shift para zona, prévia e resistência a partir de `bf2049a`, com a API 0.57 e a política `alt5-rc` (design.md §1).
- [x] 3.2 Religar K2 (`luaGrab`) e os itens do menu; ajustar a posição do atalho por app.
- [x] 3.3 Concentrar a soltura da barra e o rearm num único ponto sem pedido de fim (design.md §4, para a troca à paridade 0.56 ser local).
- [ ] 3.4 Desconectar o listener de Shift no `PLUGIN_EXIT`; manter a ordem verificada por `tools/test-plugin-lifecycle.sh`. **Não feito como escrito:** o listener segue o padrão `static` dos outros listeners de `main.cpp` (destruídos no `dlclose`; nenhum evento de teclado roda entre `PLUGIN_EXIT` e `dlclose`); o hook de movimento sai com o HANDLE. Unload durante drag fica no roteiro ao vivo.
- [x] 3.5 Documentar o débito do `floatWindow` do Lua (decisão 1.2) em `docs/DEVELOPMENT_STATE.md` e incluir no roteiro ao vivo um cenário que o mostre (float pelo menu durante SUPER+arrastar); não mudar o núcleo.

## 4. Verde e mutações

- [x] 4.1 Itens 2.1–2.6 verdes; `tools/test-plugin-input.sh` e as suítes Lua de encaixe/Assist continuam verdes.
- [x] 4.2 Mutações de cada regra (pedido de fim reintroduzido, posse em recusa, barra tomando press durante drag, soltura cancelada, rearm com fim, guarda removida, item do menu removido), mortas por assertiva.
- [x] 4.3 Build isolado GCC/Clang, `tools/check.sh` isolado e inspeção ELF (scripts da T7).

## 5. Documentar e entregar

- [x] 5.1 `docs/API.md`: `grab` disponível de novo. `docs/CONFIGURAVEL.md`: efeitos de `snap_top_edge`, `snap_preview`, `seam_resistance` e `preview_color` de volta; menu completo.
- [x] 5.2 Roteiro ao vivo em bash no `docs/STATUS.md`: drag, Shift/zona/Assist, prévia, divisa de monitores, maximizada, menu Mover/Redimensionar, drag alheio por SUPER+arrastar, captura na soltura (C1/C2), unload durante drag; carga do plugin e do núcleo primeiro; desfazer.
- [x] 5.3 `docs/DEVELOPMENT_STATE.md`: resultado, custo C1/C2, alternativa paridade 0.56 e gatilho de troca.
- [ ] 5.4 Revisão independente, se o dono quiser; commit só com aprovação.

## Notas da implementação (01/11/2026)

- 2.1–2.7: vermelho em `docs/port-0-57/evidence/A1-tests/red-{input,menu}.log` (22 falhas por assertiva no esqueleto de API com comportamento alpha-0, 12 estruturais, teste Rust).
- 3.1: o unsnap anima por 140 ms **antes** do início do drag (`KwIn::SUnsnap` em `barInput.hpp`); a 0.56.2 animava com o drag pausado (pedido de fim), o que a alt5-rc não permite. Primeiro feito sem animação; o dono pediu a animação em 01/11/2026.
- Encaixe ao soltar: roda no sinal `ended` do controlador de drag (depois do fim nativo), por um listener que pertence à barra.
- Spec: o requisito "Mover e Redimensionar pelo menu na alpha-1" teve o texto da recusa corrigido (o grab foca a janela antes do início, como em `bf2049a`).
- Ao vivo (dono, 01/11/2026): roteiros A1, A1b (animação) e A1c (C11, dois monitores) passaram.
- 5.4: commit aprovado pelo dono em 01/11/2026; revisão independente não feita.

## Arquivamento (01/11/2026)

Arquivada pelo dono depois do teste ao vivo: roteiros A1, A1b e A1c do `docs/STATUS.md` passaram em 01/11/2026. Implementação em `38cc338`, testes ao vivo documentados em `622acc8`. Specs principais `title-bar` e `snap-assist` atualizadas com os deltas desta mudança (RENAMED, REMOVED, MODIFIED, ADDED, nesta ordem), copiados como estão; sem `openspec validate` (CLI não instalada). Conferência estrutural: `tools/test-openspec-specs.sh`.

Ressalvas aceitas: a 3.4 **não foi feita como escrita** (o listener de Shift segue `static`, como os outros de `main.cpp`) e a 5.4, **revisão independente, não foi feita**. Custo C1/C2: se um arrasto preso aparecer no uso, é o gatilho para trocar para a paridade 0.56, o que exige nova proposta OpenSpec.
