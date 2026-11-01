# Tasks

Estado em 01/11/2026: proposta **aprovada pelo dono**, com os padrões das Open Questions. Nada implementado. A implementação exige autorização própria dos arquivos. Critérios em design.md, Validation Plan.

## 1. Pré-requisitos

- [x] 1.1 Aprovação do dono para esta proposta e para as Open Questions 1 e 2 (01/11/2026: padrões aceitos; não restaurar `restores`; erro do `grab` não exibido no menu).
- [ ] 1.2 Confirmar base Git e arquivos autorizados: `plugin/barInput.hpp`, `plugin/barDeco.{cpp,hpp}`, `plugin/main.cpp`, `plugin/test_input.cpp`, `tools/test-plugin-input.sh`, script de mutações novo em `docs/port-0-57/evidence/`, `docs/API.md`, `docs/STATUS.md`, `docs/DEVELOPMENT_STATE.md`.

## 2. Vermelho primeiro (host-free)

- [ ] 2.1 `startGate`: as 4 combinações (design.md §1).
- [ ] 2.2 `SUnsnap::abort()`: depois dele, nenhum passo muda a geometria e `active` fica desligado.
- [ ] 2.3 Sequências com `SPairs`: soltura liberada no YIELD, tomada no NOT_OVER.
- [ ] 2.4 `grabStartError`: as 8 combinações; mensagens distintas por causa.
- [ ] 2.5 Estrutural (ignorando comentários): `startGate(` antes de `applyUnsnap(`, `fullscreenWindow(`, `fullWindowFocus(` e `KwDrag::begin(` em cada rota de `barDeco.cpp`; `luaGrab` sem `return 0` fora do sucesso.
- [ ] 2.6 Demonstrar o vermelho de cada item contra `ae1f250` por assertiva, não por erro de compilação (esqueleto de API sem comportamento, como na alpha-1).

## 3. Implementação

- [ ] 3.1 `barInput.hpp`: `eStartGate`, `startGate`, `SUnsnap::abort()`, `grabStartError`.
- [ ] 3.2 `barDeco.cpp`: porta antes do passo do unsnap, da saída de maximizada, do `startUnsnap` e no começo de `handleMovement`; YIELD e NOT_OVER como em design.md §2–3. Rearm pela mesma porta, sem mudar comportamento.
- [ ] 3.3 `main.cpp` (`luaGrab`): erro Lua em toda saída sem drag.

## 4. Verde e mutações

- [ ] 4.1 Itens 2.1–2.5 verdes; `tools/test-plugin-input.sh` inteiro verde.
- [ ] 4.2 Script de mutações com controle, saída guardada e morte só por linha `FALHOU -` que não seja de compilação (design.md, Validation Plan).
- [ ] 4.3 Build isolado GCC/Clang, `tools/check.sh` isolado e inspeção ELF (scripts da T7).

## 5. Documentar e entregar

- [ ] 5.1 `docs/API.md`: erros de `grab` (janela não achada, foco recusado, início recusado).
- [ ] 5.2 Roteiro ao vivo em bash no `docs/STATUS.md`: carga do plugin e do núcleo primeiro; lado a lado com movimento rápido para a vizinha; drag, Shift, encaixe e unsnap normais; desfazer.
- [ ] 5.3 `docs/DEVELOPMENT_STATE.md`: resultado e limites (achado 1 coberto só host-free e por estrutura).
- [ ] 5.4 Commit só com aprovação do dono.
