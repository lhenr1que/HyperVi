# Proposal — Stabilize Show Desktop restoration

## Why

A interface possui uma operação Mostrar área de trabalho, mas a auditoria anterior identificou perda de proveniência de janelas após reload. As decisões D-43/D-44/D-45 exigem restauração automática após saída do workspace, mapeamento de nova janela elegível ou ativação pela dock de janela temporariamente ocultada no mesmo escopo, sem perder minimização, modos e geometria.

## What Changes

- Garantir o encerramento da revelação temporária quando o workspace ativo do **mesmo monitor** muda (D-43).
- Encerrar revelação quando nova janela elegível for realmente mapeada no workspace revelado, restaurando janelas antes visíveis (D-44).
- Ao ativar explicitamente na dock uma janela temporariamente ocultada, restaurar todas as janelas anteriormente visíveis e focar a escolhida (D-45). O clique não deve minimizá-la por causa da regra normal do Floating.
- Preservar, revalidar e recuperar proveniência das janelas temporariamente ocultas, inclusive em reload e falhas parciais.
- Impedir ações obsoletas/duplicadas que alterem minimização, layout ou janelas de outros monitores.
- Disponibilizar observabilidade suficiente para distinguir operação solicitada, efetivada, pendente e falha.
- Acrescentar testes de sequência, multimonitor, idempotência e regressão, aproveitando a base Lua/C++/Rust atual.

## Capabilities

### New Capabilities

Nenhuma presumida. O código já tem Mostrar área de trabalho; revisar a organização de specs reais antes de declarar uma capacidade nova.

### Modified Capabilities

- **`show-desktop` (CAMINHO PROVISÓRIO):** adicionar requisitos de saída de workspace, preservação de estado e recuperação. **ANTES DE COPIAR:** inspecionar `openspec/specs/` real e substituir pelo caminho exato da capacidade existente; se inexistente, classificá-la como nova e ajustar `Purpose` no delta.

## Impact

- Código Lua de políticas e estado de janela; eventuais adaptadores Hyprland/eventos; interface Rust/GTK4 somente para controle/diagnóstico que já couber no escopo; plugin C++ somente se necessário após auditoria.
- Eventos e consultas de monitor/workspace/janela; persistência ou outro mecanismo robusto de proveniência.
- O comportamento de nova janela passa a ser definido pela D-44. P-17.2c fica resolvida para clique explícito na dock (D-45). Permanecem fora do escopo Rice Store, VXWM, formato visual do overview e regras de foco de outros controles não aprovadas.

## References / status

- Produto ainda sem nome público aprovado.
- D-36 a D-45 e contrato `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.4.md`.
- Auditoria histórica do repositório local: commit `60e19df`; **não verificada novamente**.
- Artefatos deste diretório são **minuta para integração posterior**, não mudança criada ou validada no repositório do usuário.
