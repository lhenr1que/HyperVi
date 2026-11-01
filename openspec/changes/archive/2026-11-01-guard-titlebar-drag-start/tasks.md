# Tasks

Estado em 01/11/2026: proposta **aprovada pelo dono**, com os padrões das Open Questions. Implementação feita (o dono mandou prosseguir) e **commitada com aprovação do dono antes do teste ao vivo**; teste ao vivo pendente. Critérios em design.md, Validation Plan.

## 1. Pré-requisitos

- [x] 1.1 Aprovação do dono para esta proposta e para as Open Questions 1 e 2 (01/11/2026: padrões aceitos; não restaurar `restores`; erro do `grab` não exibido no menu).
- [x] 1.2 Confirmar base Git e arquivos autorizados: `plugin/barInput.hpp`, `plugin/barDeco.{cpp,hpp}`, `plugin/main.cpp`, `plugin/test_input.cpp`, `tools/test-plugin-input.sh`, script de mutações novo em `docs/port-0-57/evidence/`, `docs/API.md`, `docs/STATUS.md`, `docs/DEVELOPMENT_STATE.md`.

## 2. Vermelho primeiro (host-free)

- [x] 2.1 `startGate`: as 4 combinações (design.md §1).
- [x] 2.2 `SUnsnap::abort()`: depois dele, nenhum passo muda a geometria e `active` fica desligado.
- [x] 2.3 Sequências com `SPairs`: soltura liberada no YIELD, tomada no NOT_OVER.
- [x] 2.4 `grabStartError`: as 8 combinações; mensagens distintas por causa.
- [x] 2.5 Estrutural (ignorando comentários): `startGate(` antes de `applyUnsnap(`, `fullscreenWindow(`, `fullWindowFocus(` e `KwDrag::begin(` em cada rota de `barDeco.cpp`; `luaGrab` sem `return 0` fora do sucesso.
- [x] 2.6 Demonstrar o vermelho de cada item contra `ae1f250` por assertiva, não por erro de compilação (esqueleto de API sem comportamento, como na alpha-1).

## 3. Implementação

- [x] 3.1 `barInput.hpp`: `eStartGate`, `startGate`, `SUnsnap::abort()`, `grabStartError`.
- [x] 3.2 `barDeco.cpp`: porta antes do passo do unsnap, da saída de maximizada, do `startUnsnap` e no começo de `handleMovement`; YIELD e NOT_OVER como em design.md §2–3. Rearm pela mesma porta, sem mudar comportamento.
- [x] 3.3 `main.cpp` (`luaGrab`): erro Lua em toda saída sem drag.

## 4. Verde e mutações

- [x] 4.1 Itens 2.1–2.5 verdes; `tools/test-plugin-input.sh` inteiro verde.
- [x] 4.2 Script de mutações com controle, saída guardada e morte só por linha `FALHOU -` que não seja de compilação (design.md, Validation Plan).
- [x] 4.3 Build isolado GCC/Clang, `tools/check.sh` isolado e inspeção ELF (scripts da T7).

## 5. Documentar e entregar

- [x] 5.1 `docs/API.md`: erros de `grab` (janela não achada, foco recusado, início recusado).
- [x] 5.2 Roteiro ao vivo em bash no `docs/STATUS.md`: carga do plugin e do núcleo primeiro; lado a lado com movimento rápido para a vizinha; drag, Shift, encaixe e unsnap normais; desfazer.
- [x] 5.3 `docs/DEVELOPMENT_STATE.md`: resultado e limites (achado 1 coberto só host-free e por estrutura).
- [x] 5.4 Commit só com aprovação do dono (aprovado em 01/11/2026, antes do teste ao vivo GS).

## Notas da implementação (01/11/2026)

- Evidências em `docs/port-0-57/evidence/GS-tests/`. Vermelho: `red-input-{g++,clang++}.log` (esqueleto de API sem comportamento; 18 assertivas e 7 checagens de ordem falham, nenhuma por compilação). Verde: `green-input-*.log`, 152 ok nos dois compiladores.
- Desvio do design §1: nas rotas que mexem em geometria (passo do unsnap, saída de maximizada, `startUnsnap`), a porta olha só o drag ativo (`pointerOnOwnWindow = true`). Essas rotas trazem a janela para baixo do ponteiro; exigir a janela ali abortaria qualquer unsnap rápido. A janela é conferida no começo de `handleMovement`.
- Acréscimo: a porta também guarda a soltura no meio do unsnap (`onMouseButton`), que muda geometria e não estava listada.
- Limite: na soltura, o par só é liberado se o listener da barra rodar antes do listener de soltura do `main.cpp` (a hyprutils chama na ordem de registro, e o do `main.cpp` provavelmente vem antes). A geometria fica parada de qualquer jeito. O caso exige que o outro drag comece sem nenhum movimento antes da soltura.
- 2.5: o `present` antigo `KwIn::grabRefused()` em `main.cpp` virou `KwIn::grabStartError(` (a chamada mudou de lugar; o oráculo novo é a assertiva e a checagem de ordem).
- 4.2: `run-mutations.sh` com controle; **10/10 mortas por assertiva** (7 por assertiva executada, 3 pela checagem de ordem), `mutations.log`. **Limite conhecido, não contado:** neutralizar a porta em `barDeco.cpp` mantendo o texto (`if (false && ...)`) sobrevive, porque essas rotas não rodam sem o compositor. Cobertura só ao vivo.
- 4.3: build isolado GCC/Clang com 0 erros, avisos 7/2 (iguais à alpha-1); ELF passa nos dois (`elf-*.log`, `GS-sha256.txt`); `check.sh` exit 0 (`check.log`).
- 5.2: roteiro **GS** no `docs/STATUS.md`. A mensagem de erro do `hyprctl eval` no passo 4 é a esperada pelo código, ainda não vista ao vivo.

## Arquivamento (01/11/2026)

Arquivada à mão (CLI `openspec` não instalada) com aprovação do dono, depois do teste ao vivo: roteiros GS e MX do `docs/STATUS.md` passaram em 01/11/2026. Implementação em `5944fe1`; correção do cursor ao sair da maximização (achado no GS) em `849bf16`. Os 3 requisitos MODIFIED do delta substituíram os do spec principal `title-bar`, com o texto copiado como está; `snap-assist` não muda. Conferência estrutural: `tools/test-openspec-specs.sh`, que passou a conferir também esta mudança. A observação do GS passo 1 (janela em tiling centrada no cursor) é comportamento do Hyprland (`binds:drag_center_window`) e ficou como está.

