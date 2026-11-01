## ADDED Requirements

### Requirement: Contexto da escolha do Assist
O sistema SHALL recusar a escolha se workspace, monitor ou layout mudou desde a apresentação do Assist.

#### Scenario: Contexto alterado
- **WHEN** o usuário escolhe uma janela depois de mudar o workspace, o monitor ou o layout
- **THEN** a escolha não encaixa a janela
