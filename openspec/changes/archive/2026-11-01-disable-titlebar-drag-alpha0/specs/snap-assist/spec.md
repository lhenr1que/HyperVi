# snap-assist Delta Specification

## MODIFIED Requirements

### Requirement: Assist depois de encaixar numa zona
O sistema SHALL abrir o Snap Assist quando uma janela é encaixada numa zona do layout ativo no monitor em que ela já está, seja pelo menu da janela, seja com Shift ao arrastar, desde que haja zona vazia e o Assist esteja ligado. Na alpha-0 do port para Hyprland 0.57, o gatilho Shift+drag pela decoração do plugin SHALL estar indisponível conforme `title-bar`; ações explícitas de encaixe pelo menu/teclado SHALL continuar seguindo as condições de Assist, sem inventar uma soltura da barra.

#### Scenario: Shift ao arrastar no mesmo monitor fora da alpha-0
- **WHEN** fora da alpha-0 a janela é solta com Shift sobre a zona 1 do mesmo monitor, o layout tem mais de uma zona e há outras janelas flutuantes soltas no workspace
- **THEN** o Snap Assist abre sobre a próxima zona vazia

#### Scenario: Tentativa de Shift ao arrastar na alpha-0
- **WHEN** na alpha-0 o usuário tenta arrastar a janela pela decoração com Shift sobre uma zona
- **THEN** essa interação não encaixa a janela nem abre o Snap Assist

#### Scenario: Pelo menu
- **WHEN** a janela é encaixada na zona 1 pelo menu da janela, inclusive na alpha-0
- **THEN** o Snap Assist abre sobre a próxima zona vazia

#### Scenario: Assist desligado
- **WHEN** o Snap Assist está desligado nas configurações
- **THEN** a janela é encaixada por uma ação explícita e nada mais abre
