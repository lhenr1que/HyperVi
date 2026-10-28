# title-bar Specification

## Purpose

A barra de título que o plugin `kwinish-bars` põe nas janelas, no estilo do KWin: os botões, as ações deles e o retorno que dão ao passar o mouse.

## Requirements

### Requirement: Dica nos botões de minimizar, maximizar e fechar
O sistema SHALL mostrar uma dica com o nome da ação quando o ponteiro fica parado sobre um botão da barra cuja ação é `minimize`, `maximize` ou `close`, depois de um atraso curto (menos de um segundo). A dica fica por cima das janelas, logo abaixo do botão, inteira dentro do monitor do botão. Botões com outras ações não mostram dica.

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
- **WHEN** o ponteiro fica parado sobre o botão de fixar ou sobre um botão `exec:`
- **THEN** nenhuma dica aparece

### Requirement: Texto da dica no idioma do ambiente
O sistema SHALL escrever a dica em português quando o locale do compositor (o primeiro valor não vazio entre `LC_ALL`, `LC_MESSAGES` e `LANG`) começa por `pt`, e em inglês nos outros casos: Minimizar/Minimize, Maximizar/Maximize, Fechar/Close. No botão de maximizar de uma janela já maximizada o texto SHALL ser Restaurar/Restore.

#### Scenario: Locale português
- **WHEN** `LANG=pt_BR.UTF-8` e o ponteiro para no botão de minimizar
- **THEN** a dica diz "Minimizar"

#### Scenario: Locale inglês ou ausente
- **WHEN** nenhuma das variáveis começa por `pt` e o ponteiro para no botão de maximizar de uma janela não maximizada
- **THEN** a dica diz "Maximize"

#### Scenario: Janela maximizada
- **WHEN** a janela está maximizada e o ponteiro para no botão de maximizar
- **THEN** a dica diz "Restaurar" (ou "Restore")

### Requirement: A dica some quando deixa de valer
O sistema SHALL tirar a dica da tela quando o ponteiro sai do botão, quando um botão do mouse é apertado na barra, quando começa um arrasto, quando a barra é desligada ou escondida, quando a janela fecha e quando o plugin é descarregado.

#### Scenario: Sair do botão
- **WHEN** a dica está visível e o ponteiro sai do botão
- **THEN** a dica some no quadro seguinte

#### Scenario: Clicar no botão
- **WHEN** a dica do maximizar está visível e o usuário clica no botão
- **THEN** a dica some e a ação do botão acontece como antes

#### Scenario: Descarregar o plugin com a dica na tela
- **WHEN** o plugin é descarregado enquanto uma dica está visível ou esperando o atraso
- **THEN** a dica some e o compositor continua funcionando
