# workspace-modes Specification

## Purpose

Cada workspace tem um modo escolhido pelo usuário (tiling, scrolling ou floating), lembrado entre sessões; o modo decide como as janelas desse workspace são dispostas quando abrem, quando o modo muda e quando o modo termina.

## Requirements

### Requirement: Janela nova nasce flutuante no modo floating
O sistema SHALL exibir uma janela que abre num workspace no modo floating já flutuante desde o primeiro quadro em que ela aparece, com o tamanho do modo: 64% da largura e 68% da altura da área útil do monitor (no máximo 1100 x 760), centralizada, com um pequeno deslocamento em cascata entre janelas abertas em sequência. A janela MUST NOT aparecer, nem por um quadro, com o tamanho que teria em mosaico.

#### Scenario: Aplicativo aberto num workspace floating
- **WHEN** o usuário abre um aplicativo num workspace no modo floating
- **THEN** a janela aparece flutuante, centralizada na área útil, com 64% x 68% dela (no máximo 1100 x 760), e não passa pela área útil inteira

#### Scenario: Duas janelas abertas em sequência
- **WHEN** duas janelas abrem uma depois da outra no mesmo workspace floating
- **THEN** a segunda aparece deslocada em cascata em relação à primeira, as duas já flutuantes

#### Scenario: Workspace em mosaico
- **WHEN** o usuário abre um aplicativo num workspace no modo tiling ou scrolling
- **THEN** a janela entra no layout de mosaico daquele modo, como antes

### Requirement: O modo floating só reivindica as janelas que ele fez flutuar
O sistema SHALL marcar como "flutuante pelo modo" apenas as janelas que o modo floating fez flutuar, inclusive as que nasceram flutuantes por ele, e ao trocar o workspace para tiling ou scrolling SHALL devolver ao mosaico somente essas. Janelas que já abrem flutuantes por conta própria (por exemplo, diálogos) ou fixadas em todas as workspaces MUST NOT ser marcadas nem alteradas pelo modo.

#### Scenario: Sair do modo depois de abrir janelas nele
- **WHEN** janelas abertas num workspace floating nasceram flutuantes e o usuário muda o workspace para tiling
- **THEN** essas janelas voltam ao mosaico

#### Scenario: Diálogo que já nasce flutuante
- **WHEN** um diálogo que o próprio aplicativo pede flutuante abre num workspace floating e depois o workspace muda para tiling
- **THEN** o diálogo fica como o aplicativo pediu, flutuante, e não é posto no mosaico

#### Scenario: Janela que abre em tela cheia
- **WHEN** uma janela pede tela cheia ao abrir num workspace floating e depois sai da tela cheia
- **THEN** ela fica em tela cheia enquanto pedir e, ao sair, aparece flutuante no retângulo do modo, não em mosaico
