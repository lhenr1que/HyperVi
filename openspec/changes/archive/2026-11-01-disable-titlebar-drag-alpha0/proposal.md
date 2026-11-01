# Proposal

## Why

O port para Hyprland 0.57 não tem identidade pública de sessão de drag que permita encerrar com segurança uma aquisição da barra. Q4 continua bloqueada: retirar somente os fins mantém inícios e cancelamentos que prendem operações. O plano de continuidade de 30/10/2026 (§4–5, T3/T4) escolheu D1 completo para a alpha-0. Esta proposta formaliza a redução temporária; sua aprovação, implementação T4 e validação são etapas distintas.

## What Changes

- Na alpha-0, excluir da compilação as rotas de drag da decoração: K1/K2, flags de aquisição/pendência, rearm, unsnap, snap-on-release, preview e resistência associados, arrasto por toque e cancelamentos exclusivos dessas rotas. Inventário fechado em design.md.
- Preservar os cliques de botões, ícone e menu com press/release pareados, sem aquisição nem fim de drag pelas rotas desativadas.
- Ocultar Mover/Redimensionar em `ui/src/menu.rs`. A entrada Lua `grab` mantém nome e argumentos, mas informa indisponibilidade na alpha-0 sem focar, adquirir ou engolir cliques; não será uma operação silenciosa oferecida pelo menu.
- Movimento interativo de janela fica pelo bind nativo configurado no Hyprland; não criar bind nem alterar a configuração do dono. Encaixes e deslocamentos discretos já existentes continuam.
- Drag fica para alpha-1, após decisão Q4/T10 e recheck de `target()` em `KwDrag::begin` (T9), com autorização e revisão próprias.

## Capabilities

### New Capabilities

Nenhuma.

### Modified Capabilities

- `title-bar`: indisponibilidade temporária de drag, input pareado e menu reduzido na alpha-0.
- `snap-assist`: explicitar que o gatilho por Shift+drag não existe na alpha-0; menu e ações explícitas continuam.

## Impact

T4 poderá alterar `plugin/barDeco.cpp`, `plugin/main.cpp`, declarações/estado correspondentes em `plugin/barDeco.hpp` e `plugin/globals.hpp`, `ui/src/menu.rs` e testes host-free. Não altera `hostDrag.hpp`, grupo R, `PLUGIN_EXIT`, técnica GL, formatos persistentes ou binds nativos. Não estende a exclusão às barras fallback de `ui/src/bars.rs`; a alpha-0 aqui é a decoração do plugin carregado.

`docs/API.md` §§2–3 tem a assinatura de `grab`; a indisponibilidade temporária é mudança semântica de disponibilidade, a documentar em T4. `docs/CONFIGURAVEL.md` §§1/4 precisará indicar menu reduzido e opções de drag sem efeito nessa alpha, sem remover opções nem mudar seus valores/formato. Ambos ficam intactos em T3: descrevem implementação atual e estão fora dos caminhos de escrita autorizados. A proposta não cria opção de ativação runtime.

Fora de escopo: implementação, build, sincronização/arquivamento de specs, revisão independente nesta sessão, grupo R e contenção de `floatWindow` (inclusive Lua). D1 não promete segurança O1 para todas as ações integradas do Kwinish.
