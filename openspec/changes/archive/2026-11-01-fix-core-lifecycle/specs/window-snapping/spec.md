## ADDED Requirements

### Requirement: Zonas respeitam elegibilidade da janela
O sistema SHALL ignorar pedidos de zona para janelas não mapeadas ou em tela cheia, como faz o encaixe pelo teclado.

#### Scenario: Zona em janela inelegível
- **WHEN** uma ação de zona aponta para uma janela não mapeada ou em tela cheia
- **THEN** sua geometria não é alterada e o Assist não é iniciado
