# window-snapping Specification

## Purpose

Encaixar janelas flutuantes em regiões fixas da área útil do monitor (metades, quartos, tela inteira), a partir do teclado, do menu da janela ou do arrasto pela barra de título.

## Requirements

### Requirement: Encaixe pelo teclado
O sistema SHALL encaixar a janela ativa na região pedida quando um atalho de encaixe é usado, calculando a região na área útil do monitor em que a janela está.

#### Scenario: Atalho de metade esquerda numa janela flutuante
- **WHEN** a janela ativa é flutuante, não está em tela cheia, e o usuário aperta o atalho de "metade esquerda"
- **THEN** a janela ocupa a metade esquerda da área útil do seu monitor, sem erro

#### Scenario: Atalho repetido restaura
- **WHEN** o mesmo atalho é usado de novo na janela já encaixada
- **THEN** a janela volta ao tamanho e posição de antes do encaixe

#### Scenario: Janela ladrilhada ou em tela cheia
- **WHEN** a janela ativa é ladrilhada ou está em tela cheia
- **THEN** nada muda e nenhum erro ocorre

### Requirement: Monitor do soltar
O sistema SHALL usar o monitor indicado na ação (o monitor em que a janela foi solta) quando ele existir, e o monitor da janela quando não existir ou for desconhecido.

#### Scenario: Soltar no outro monitor
- **WHEN** o encaixe chega como `tile:left@<monitor>` com um monitor existente
- **THEN** a região é calculada nesse monitor

#### Scenario: Monitor desconhecido
- **WHEN** o encaixe chega com um nome de monitor que não existe
- **THEN** a região é calculada no monitor da própria janela

### Requirement: Zonas respeitam elegibilidade da janela
O sistema SHALL ignorar pedidos de zona para janelas não mapeadas ou em tela cheia, como faz o encaixe pelo teclado.

#### Scenario: Zona em janela inelegível
- **WHEN** uma ação de zona aponta para uma janela não mapeada ou em tela cheia
- **THEN** sua geometria não é alterada e o Assist não é iniciado
