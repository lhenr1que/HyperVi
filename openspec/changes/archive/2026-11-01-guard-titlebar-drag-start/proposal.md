# Proposal

## Why

A revisão independente da alpha-1 (Codex, 01/11/2026, conferida pelo orquestrador; relatório fora do repositório, `A1-review-codex-2026-11-01.md`) achou três falhas no início do arrasto pela barra, todas confirmadas por leitura de `38cc338`:

1. **O unsnap não cede a outro drag** (achado 1, major). Durante a saída de encaixe (140 ms), o início adiado e a saída de maximizada, a barra muda geometria e foco sem conferir se já há drag ativo. O contrato `alt5-rc` (`docs/port-0-57/evidence/Q4A1-alt5-contract.md`, §2, linha 6) manda abortar o unsnap nesse caso.
2. **O início adiado pode arrastar outra janela** (achado 2, preexistente, contrato §7 "Hit-test"). `KwDrag::begin` escolhe a janela sob o ponteiro; `handleMovement` não confere se ela é a da barra. Um movimento rápido para a vizinha arrasta a vizinha, e a barra perde o encaixe ao soltar.
3. **Alguns grabs que não começam devolvem sucesso** (achado 3). Em `luaGrab`, endereço inexistente, janela oculta ou foco recusado retornam sem erro. O spec pede diagnóstico. O menu chama `grab` dentro de `pcall` e descarta o erro (`core/kwinish.lua:40`), então a correção não muda nada visível no menu; vale para o contrato da API Lua.

Nenhum dos três prende o arrasto. Todos os testes ao vivo da alpha-1 passaram (A1, A1b, A1c).

## What Changes

- Antes de qualquer mudança de geometria ou foco ligada ao início do drag da barra (passo do unsnap, saída de maximizada, início adiado, `handleMovement`), a barra confere se há drag ativo. Se houver, **aborta**: não mexe mais na geometria nem no foco, não inicia, limpa o início pendente e o unsnap, e deixa a soltura do press chegar ao compositor.
- O início do drag da barra só acontece com a própria janela da barra sob o ponteiro, pelo mesmo hit-test que `KwDrag::begin` usa. Fora dela, não há início, foco nem estado armado; o press segue como clique.
- `grab` devolve erro Lua sempre que o drag não começa: janela não achada, oculta ou não mapeada; foco recusado; início recusado.
- As decisões novas ficam em `plugin/barInput.hpp` (host-free) e são testadas por assertiva, vermelho primeiro.

## Capabilities

### New Capabilities

Nenhuma.

### Modified Capabilities

- `title-bar`: "Arrasto pela barra com fim pelo compositor" (início só na janela da barra), "Encaixe, prévia e resistência ao arrastar pela barra" (saída de encaixe ou de maximizada aborta com drag ativo) e "Mover e Redimensionar pelo menu na alpha-1" (todo grab que não começa é erro Lua).

## Impact

Código: `plugin/barInput.hpp`, `plugin/barDeco.{cpp,hpp}`, `plugin/main.cpp` (`luaGrab`). Testes: `plugin/test_input.cpp`, `tools/test-plugin-input.sh`, um script de mutações novo. Docs: `docs/API.md` (erros de `grab`), roteiro ao vivo no `docs/STATUS.md`, `docs/DEVELOPMENT_STATE.md`. `plugin/hostDrag.hpp`, o núcleo Lua e a UI não mudam. Sem opção nova e sem formato persistente.

A política de fim continua `alt5-rc`: nada aqui pede o fim de um drag.

Fora de escopo (registrados, não iniciados): achado 6 da revisão (clique simples em janela flutuante inicia drag no press; divergência entre spec e código, à espera de decisão do dono); endurecer `run-mutations.sh` da alpha-1 e as assertivas tautológicas `drags(c) == 0` (achados 4 e 5); expor ao usuário o erro do `grab` no menu; pendências antigas (Q1 por log, `SWindowRenderPresentation`, logs P5–T6, `floatWindow` do Lua, C1/C2).
