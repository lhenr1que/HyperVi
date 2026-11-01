# snap-assist Delta Specification

## MODIFIED Requirements

### Requirement: Assist depois de encaixar numa zona
O sistema SHALL abrir o Snap Assist quando uma janela é encaixada numa zona do layout ativo no monitor em que ela já está, seja pelo menu da janela, seja com Shift ao arrastar pela barra, desde que haja zona vazia e o Assist esteja ligado. Na alpha-1 do port para Hyprland 0.57, o gatilho Shift+drag pela decoração do plugin SHALL voltar a valer.

#### Scenario: Shift ao arrastar no mesmo monitor
- **WHEN** a janela é solta com Shift sobre a zona 1 do mesmo monitor, o layout tem mais de uma zona e há outras janelas flutuantes soltas no workspace
- **THEN** o Snap Assist abre sobre a próxima zona vazia

#### Scenario: Pelo menu
- **WHEN** a janela é encaixada na zona 1 pelo menu da janela
- **THEN** o Snap Assist abre sobre a próxima zona vazia

#### Scenario: Assist desligado
- **WHEN** o Snap Assist está desligado nas configurações
- **THEN** a janela é encaixada e nada mais abre
