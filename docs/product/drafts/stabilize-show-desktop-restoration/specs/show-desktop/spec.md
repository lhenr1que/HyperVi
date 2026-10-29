# Spec Delta — Show Desktop restoration

> **Atenção:** caminho da capability `show-desktop` é provisório. Consultar a spec real antes de importar. Se a capability já existir, manter apenas requisitos novos sob ADDED e fazer MODIFIED somente copiando o requisito original integralmente.

## ADDED Requirements

### Requirement: Restaurar ao deixar o workspace revelado
O gerenciador DEVE encerrar a ocultação temporária das janelas elegíveis quando o workspace ativo do mesmo monitor mudar, restaurando seu estado anterior sem modificar outros workspaces.

#### Scenario: Trocar workspace no mesmo monitor
- **WHEN** o workspace A2 está revelado por Mostrar área de trabalho e o usuário ativa A3 no monitor A
- **THEN** as janelas antes visíveis de A2 são restauradas e A3/B5 permanecem inalterados

#### Scenario: Alterar somente o monitor em foco
- **WHEN** A2 está revelado e o foco passa para o monitor B sem alterar o workspace ativo de A
- **THEN** A2 permanece revelado e não ocorre restauração por saída

### Requirement: Preservar minimização anterior
O gerenciador DEVE distinguir janelas anteriormente minimizadas das temporariamente ocultadas e restaurar somente as afetadas pela operação Mostrar área de trabalho.

#### Scenario: Janela já minimizada
- **WHEN** A2 possui uma janela previamente minimizada e Mostrar área de trabalho é encerrado ao ativar A3
- **THEN** a janela previamente minimizada continua minimizada após retornar a A2

### Requirement: Preservar organização e proveniência
A restauração DEVE preservar, dentro das capacidades do compositor, o workspace lógico, modo de gerenciamento, estado de floating e geometria das janelas elegíveis.

#### Scenario: Modos e geometria
- **WHEN** um workspace Floating, Tiling ou Scroll é revelado e depois abandonado
- **THEN** retornar a esse workspace não altera seu modo e mantém os atributos da disposição anterior

### Requirement: Evitar restaurar referências obsoletas
A restauração DEVE validar janela, sessão, destino e operação antes de agir, ignorando janelas fechadas e evitando atuar sobre identificadores reutilizados.

#### Scenario: Janela fechada durante a ocultação
- **WHEN** uma janela ocultada é fechada antes do encerramento da revelação
- **THEN** nenhuma operação de restauração altera uma nova janela ou outro destino com identificador reutilizado

### Requirement: Suportar repetição e atraso de eventos
O processamento de saída DEVE ser idempotente por geração de operação e revalidar o estado efetivo frente a eventos duplicados ou atrasados.

#### Scenario: Dupla notificação de saída
- **WHEN** o gerenciador recebe duas notificações sobre a mesma mudança de workspace
- **THEN** cada janela afetada é restaurada no máximo uma vez e não ocorre nova ocultação

### Requirement: Recuperar após reload e falhas parciais
O gerenciador DEVE permitir reconciliar o estado de janelas ocultas após reload ou falha parcial, impedindo que fiquem irrecuperavelmente ocultas.

#### Scenario: Reload durante Mostrar área de trabalho
- **WHEN** o núcleo é recarregado enquanto há janelas temporariamente ocultas
- **THEN** a reinicialização consegue revalidar a proveniência e restaurar ou disponibilizar recuperação segura dessas janelas

#### Scenario: Restauração parcialmente malsucedida
- **WHEN** parte das janelas não pode ser restaurada durante uma tentativa
- **THEN** o sistema mantém a indicação de pendência e não relata sucesso integral

### Requirement: Integrar observação e exploração sem ativação
A Visão Geral DEVE continuar mostrando janelas temporariamente ocultadas em seu inventário lógico, sem restaurá-las apenas porque um cartão foi examinado.

#### Scenario: Inspecionar workspace remoto
- **WHEN** A2 está revelado e o usuário abre a Visão Geral no monitor A e examina B5
- **THEN** o foco efetivo não muda, A2 não é restaurado somente pela inspeção e B5 é exibido no inventário


### Requirement: Restaurar automaticamente ao surgir nova janela elegível no workspace revelado
O gerenciador DEVE encerrar a ocultação temporária do workspace e restaurar todas as janelas anteriormente visíveis que foram ocultadas pela operação quando uma nova janela de aplicativo **elegível for efetivamente mapeada** nesse workspace, apresentando a janela nova normalmente sem restaurar as janelas já minimizadas. Deve revalidar o destino e ser idempotente em relação ao gatilho de saída de workspace.

#### Scenario: Nova janela mapeada no workspace revelado
- **WHEN** A2 está revelado e uma nova janela normal de aplicativo é efetivamente criada em A2
- **THEN** as janelas temporariamente ocultas de A2 são restauradas, as previamente minimizadas continuam minimizadas, e a nova janela fica visível no modo atual

#### Scenario: Spawn sem janela ou mapeamento fora do escopo
- **WHEN** A2 está revelado, mas um comando de lançamento falha, não cria um toplevel ou mapeia uma janela exclusivamente em B5
- **THEN** a revelação de A2 não é encerrada por esse evento

#### Scenario: Evento de saída e nova janela concorrentes
- **WHEN** A2 está revelado e o compositor entrega tanto evento de nova janela quanto evento de mudança A2→A3 para a mesma geração
- **THEN** somente uma operação de restauração por geração é aplicada, sem alternância destrutiva ou alteração de B5

#### Scenario: Superfície não elegível
- **WHEN** uma notificação ou superfície do shell aparece enquanto A2 está revelado
- **THEN** a ocultação de A2 não é encerrada apenas por essa superfície


### Requirement: Restaurar o workspace e focar janela temporariamente oculta selecionada na dock (D-45)
O gerenciador DEVE, ao receber ativação explícita da dock de um cliente previamente existente e temporariamente ocultado por Mostrar área de trabalho, encerrar a revelação de seu workspace, restaurar todas as janelas anteriormente visíveis que foram ocultadas e então tentar focar o cliente selecionado. Clientes previamente minimizados permanecem minimizados. A operação DEVE compartilhar a restauração idempotente de D-43/D-44 e validar o alvo antes do foco.

#### Scenario: Selecionar pela dock janela temporariamente oculta
- **WHEN** A2 está em Mostrar área de trabalho e o usuário clica no ícone da janela Firefox de A2 que foi temporariamente ocultada, enquanto Terminal também estava visível antes da revelação
- **THEN** Firefox e Terminal são restaurados e Firefox recebe foco sem ser minimizado por clique no Floating

#### Scenario: Preservar janela previamente minimizada
- **WHEN** A2 tem uma janela Música minimizada previamente à revelação e o usuário ativa pela dock outra janela temporariamente oculta
- **THEN** Música continua minimizada enquanto as janelas que eram visíveis são restauradas e a selecionada recebe foco

#### Scenario: Não restaurar por hover ou inspeção
- **WHEN** o usuário apenas passa o cursor sobre uma prévia da dock ou inspeciona um cartão da Visão Geral
- **THEN** não há restauração automática nem foco em uma janela

#### Scenario: Evitar foco tardio incorreto
- **WHEN** A2 é restaurado por clique na dock mas ocorre mudança de workspace, fechamento da janela ou evento de geração anterior antes do foco
- **THEN** o gerenciador revalida a intenção e não transfere foco inesperadamente a um alvo obsoleto; a restauração já efetivada permanece

#### Scenario: Falha de foco após restauração
- **WHEN** a restauração das janelas termina mas o foco na janela selecionada falha
- **THEN** o workspace permanece restaurado e o sistema registra que o foco não foi concluído, sem executar nova ocultação
