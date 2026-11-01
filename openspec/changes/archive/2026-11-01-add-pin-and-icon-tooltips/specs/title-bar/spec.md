# Spec Delta

## MODIFIED Requirements

### Requirement: Dica nos botões de minimizar, maximizar e fechar
O sistema SHALL mostrar uma dica com o nome da ação quando o ponteiro fica parado sobre um botão da barra cuja ação é `minimize`, `maximize` ou `close`, depois de um atraso curto (menos de um segundo). A dica fica por cima das janelas, logo abaixo do botão, inteira dentro do monitor do botão. Botões cujas ações não têm dica nesta spec (por exemplo `exec:`, `float`, `fullscreen`) não mostram dica.

#### Scenario: Parar o mouse no fechar
- **WHEN** o ponteiro entra no botão de fechar de uma barra e fica parado nele
- **THEN** depois do atraso aparece abaixo do botão uma caixinha com "Fechar" (ou "Close")

#### Scenario: Passar rápido pelos botões
- **WHEN** o ponteiro atravessa o botão sem parar nele pelo tempo do atraso
- **THEN** nenhuma dica aparece

#### Scenario: Botão perto da borda do monitor
- **WHEN** o botão fica tão perto da borda direita ou de baixo do monitor que a dica não caberia abaixo dele
- **THEN** a dica é deslocada para caber no monitor (para a esquerda, ou para cima da barra), sem sair da tela

#### Scenario: Botão sem dica
- **WHEN** o ponteiro fica parado sobre um botão `exec:`
- **THEN** nenhuma dica aparece

### Requirement: Texto da dica no idioma do ambiente
O sistema SHALL escrever a dica em português quando o locale do compositor (o primeiro valor não vazio entre `LC_ALL`, `LC_MESSAGES` e `LANG`) começa por `pt`, e em inglês nos outros casos: Minimizar/Minimize, Maximizar/Maximize, Fechar/Close, Em todas as áreas de trabalho/On all workspaces, Menu da janela/Window menu. O texto SHALL dizer o que o clique faz: Restaurar/Restore no maximizar de uma janela maximizada, Só nesta área de trabalho/Only on this workspace no fixar de uma janela fixada.

#### Scenario: Locale português
- **WHEN** `LANG=pt_BR.UTF-8` e o ponteiro para no botão de minimizar
- **THEN** a dica diz "Minimizar"

#### Scenario: Locale inglês ou ausente
- **WHEN** nenhuma das variáveis começa por `pt` e o ponteiro para no botão de maximizar de uma janela não maximizada
- **THEN** a dica diz "Maximize"

#### Scenario: Janela maximizada
- **WHEN** a janela está maximizada e o ponteiro para no botão de maximizar
- **THEN** a dica diz "Restaurar" (ou "Restore")

#### Scenario: Janela fixada
- **WHEN** a janela está fixada em todas as áreas de trabalho e o ponteiro para no botão de fixar
- **THEN** a dica diz "Só nesta área de trabalho" (ou "Only on this workspace")

## ADDED Requirements

### Requirement: Dica no botão de fixar e no ícone do aplicativo
O sistema SHALL mostrar a dica, com o mesmo atraso, posição e regras para sumir das dicas dos outros botões, quando o ponteiro fica parado sobre um botão cuja ação é `pin` ou `menu` e sobre o ícone do aplicativo à esquerda da barra. O ícone e o botão `menu` dizem "Menu da janela" (ou "Window menu"), porque o clique abre o menu da janela.

#### Scenario: Parar o mouse no fixar
- **WHEN** a janela não está fixada e o ponteiro fica parado sobre o botão de fixar
- **THEN** depois do atraso aparece abaixo do botão "Em todas as áreas de trabalho" (ou "On all workspaces")

#### Scenario: Parar o mouse no ícone
- **WHEN** a barra mostra o ícone do aplicativo e o ponteiro fica parado sobre ele
- **THEN** depois do atraso aparece abaixo do ícone "Menu da janela" (ou "Window menu"), inteira dentro do monitor

#### Scenario: Barra sem ícone
- **WHEN** o ícone está desligado (`show_icon = false`) ou o aplicativo não tem ícone
- **THEN** parar o ponteiro onde o ícone ficaria não mostra dica
