## ADDED Requirements

### Requirement: Símbolos respeitam a escala do monitor
O sistema SHALL renderizar os símbolos dos botões para a escala do monitor em que a barra está sendo desenhada.

#### Scenario: Barras em escalas diferentes
- **WHEN** uma textura de símbolo é reutilizada em outra escala
- **THEN** o símbolo é renderizado novamente para a escala atual

## MODIFIED Requirements

### Requirement: Cliques de mouse pareados
O sistema SHALL aceitar o press somente quando a janela da barra é a janela superior sob o ponteiro. O sistema SHALL preservar botões, ícone, menu, roda, hover/tooltip, foco/elevação e duplo clique configurado. Cliques consumidos nos botões, ícone ou menu SHALL consumir press e release do mesmo botão, executando a ação uma única vez. O press no título que inicia drag é a exceção: sua soltura SHALL chegar ao compositor. Cliques não consumidos SHALL passar sem retenção. Mudança de foco ou ponteiro entre press e release MUST NOT produzir par órfão por intervenção da barra. Numa janela flutuante, não maximizada e não encaixada, o press no título SHALL iniciar o drag do compositor já no press, sem esperar o limiar; um clique sem movimento MUST NOT mover nem encaixar a janela. Numa janela lado a lado, maximizada ou encaixada, o drag SHALL começar só quando o ponteiro passa do limiar.

#### Scenario: Minimizar muda foco antes da soltura
- **WHEN** o botão minimizar consome press e o foco muda antes do release
- **THEN** a ação acontece uma vez e a soltura correspondente também é consumida

#### Scenario: Ícone ou botão direito abre menu
- **WHEN** um clique no ícone ou com botão direito na barra abre o menu, sem drag ativo
- **THEN** press e release são tratados em par mesmo se o ponteiro sair da barra

#### Scenario: Press no título vira drag
- **WHEN** um press no título passa do limiar e inicia drag
- **THEN** a barra não toma a soltura e o compositor encerra o drag com ela

#### Scenario: Clique simples no título
- **WHEN** o usuário aperta e solta o título de uma janela lado a lado, maximizada ou encaixada sem passar do limiar
- **THEN** a janela é focada (e elevada, se flutuante) e nenhum drag é iniciado

#### Scenario: Clique simples no título de janela flutuante
- **WHEN** o usuário aperta e solta, sem mover o ponteiro, o título de uma janela flutuante, não maximizada e não encaixada
- **THEN** a janela é focada e elevada, e o drag do compositor começa no press e termina na soltura
- **AND** a janela não muda de posição e não é encaixada

#### Scenario: Barra focada coberta
- **WHEN** o ponteiro está sobre outra janela que cobre a barra da janela focada
- **THEN** a barra coberta não toma o press nem muda o foco
