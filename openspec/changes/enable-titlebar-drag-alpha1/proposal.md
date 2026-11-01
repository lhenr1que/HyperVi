# Proposal

## Why

A alpha-0 do port para Hyprland 0.57 (mudança `disable-titlebar-drag-alpha0`, D1) tirou o arrasto pela barra porque o 0.57 não identifica de quem é cada sessão de drag, e encerrar sem posse cortava drags de terceiros (Q4). Os marcos 1 e 2 foram atingidos (build, check e teste ao vivo do dono em 01/11/2026). Em 01/11/2026 o dono decidiu a T10: religar o arrasto na alpha-1 com a política **`alt5-rc`** de `docs/port-0-57/evidence/Q4A1-alt5-contract.md`, aceitando o custo de liveness C1/C2, e pediu que a paridade 0.56 (`cur-0.56`) fique anotada como alternativa. O recheck de `target()` em `KwDrag::begin` (T9, pré-requisito) foi feito em `510d204`.

## What Changes

- Religar o arrasto pela barra (início K1 por `KwDrag::begin`), com encaixe ao soltar, Shift para zona, prévia, resistência na divisa entre monitores, unsnap e rearm por Shift, a partir das rotas de `bf2049a`.
- Religar Mover/Redimensionar no menu da janela (início K2 por `luaGrab`), armando o grab só quando o início devolve `BEGIN_STARTED`.
- **Política de fim `alt5-rc`:** o Kwinish nunca pede o fim de um drag (sem K3, K9 nem `endDragTarget`); a soltura passa ao compositor, que encerra o drag pelo fim nativo. A barra cede a qualquer drag já ativo, no botão esquerdo e no direito.
- Guarda de drag ativo nos `floatWindow` do plugin C++.
- Toque na barra continua desligado (fora do modelo Q4-A1).
- Documentar em `docs/API.md`/`docs/CONFIGURAVEL.md` a volta de `grab` e dos efeitos de `snap_top_edge`, `snap_preview`, `seam_resistance` e `preview_color`.

## Capabilities

### New Capabilities

Nenhuma.

### Modified Capabilities

- `title-bar`: arrasto e grab do menu voltam com fim pelo compositor; os requisitos "indisponível na alpha-0" saem; pareamento de cliques ganha a exceção da soltura do drag; toque segue desligado.
- `snap-assist`: Shift ao arrastar pela barra volta a disparar o Assist.

## Impact

Código: `plugin/barDeco.{cpp,hpp}`, `plugin/main.cpp`, `plugin/globals.hpp`, `plugin/barInput.hpp` (decisões de press/soltura), `ui/src/menu.rs` (itens Mover/Redimensionar), testes host-free e a chamada a `drawSnapPreview` (já compilada no grupo R). `plugin/hostDrag.hpp` não muda (T9 feita). Sem formato persistente novo, sem opção nova, sem bind instalado.

Dependência de arquivamento: esta mudança remove e renomeia requisitos que a D1 adiciona; a D1 SHALL ser arquivada antes desta.

Custo aceito (decisão do dono): com captura de entrada na soltura, o drag da barra fica vivo até a próxima tecla ou o próximo clique não capturado (C1/C2). K2/K3 do contrato já falhavam antes e continuam.

Fora de escopo: toque na barra, ganchos ou mudança no Hyprland (proposta upstream é opcional e separada), garantia O1 para o `floatWindow` do núcleo Lua (débito decidido pelo dono em 01/11/2026, design.md Open Questions 1), Q2, implementação, build e teste ao vivo nesta etapa.
