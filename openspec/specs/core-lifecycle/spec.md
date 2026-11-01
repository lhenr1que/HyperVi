# core-lifecycle Specification

## Purpose

O estado do núcleo Lua que precisa sobreviver a uma recarga do módulo e a mudanças rápidas de escolha: origens de "mostrar área de trabalho", colunas do modo scrolling e ganchos registrados uma vez só.

## Requirements

### Requirement: Mostrar área de trabalho sobrevive à recarga do núcleo
O sistema SHALL guardar a origem das janelas ocultadas em estado que sobreviva à recarga do módulo e restaurá-las na próxima invocação.

#### Scenario: Recarga com janelas ocultadas
- **WHEN** o usuário mostra a área de trabalho, recarrega o núcleo e invoca novamente a ação
- **THEN** as janelas ainda ocultadas voltam aos seus workspaces de origem

### Requirement: Atualização de colunas respeita a última escolha
O sistema SHALL usar a escolha de colunas vigente ao executar a atualização adiada.

#### Scenario: Escolha ou remoção durante a espera
- **WHEN** uma atualização está pendente e o usuário altera ou remove a escolha
- **THEN** a atualização usa a escolha nova ou não redimensiona se ela foi removida

### Requirement: Recarga não duplica callbacks
O sistema SHALL impedir que callbacks e timers de versões antigas do núcleo continuem a agir após uma recarga.

#### Scenario: Repetir setup e hotcorners após recarga
- **WHEN** o núcleo é recarregado e setup e hotcorners são configurados novamente
- **THEN** cada evento e cada intervalo produz apenas a ação da configuração vigente
