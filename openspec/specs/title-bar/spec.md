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

### Requirement: Drag da decoração indisponível na alpha-0
Na alpha-0 para Hyprland 0.57, o sistema SHALL manter indisponível o movimento/redimensionamento interativo pela decoração do plugin e pelo grab do menu. As rotas da barra/menu MUST NOT iniciar, rearmar, substituir ou solicitar fim de drag. O sistema MUST NOT armar aquisição ou pendência de drag, fazer unsnap por arrasto, encaixar ao soltar, mostrar preview associado ou resistir ao cruzamento de monitores por essas rotas. O movimento interativo da janela SHALL permanecer disponível pelo bind nativo configurado no Hyprland, sem instalar ou mudar binds do usuário. Ações discretas de encaixe e workspace permanecem disponíveis.

#### Scenario: Arrastar título de janela flutuante ou ladrilhada
- **WHEN** o usuário pressiona o título vazio, move o ponteiro e solta, com ou sem Shift
- **THEN** a barra não inicia/rearma/encerra drag nem aplica snap, preview ou resistência
- **AND** não mantém flag de aquisição ou gesto pendente

#### Scenario: Arrastar janela maximizada ou previamente encaixada
- **WHEN** o usuário tenta arrastar a janela pelo título
- **THEN** a tentativa não restaura tamanho, não retira maximização e não inicia unsnap
- **AND** duplo clique configurado e ações explícitas continuam disponíveis

#### Scenario: Drag nativo sem ação de floating
- **WHEN** um bind nativo do Hyprland move uma janela e passam eventos de movimento, Shift e soltura sem acionar botão/duplo clique de floating
- **THEN** as rotas desativadas da barra não alteram o delta nem adquirem, rearmam, encerram ou engolem eventos para controlar o drag

### Requirement: Toque na decoração indisponível na alpha-0
Na alpha-0, o sistema MUST NOT interpretar down/move/up de toque na decoração do plugin como gesto, clique de ação ou pedido de float, resize, pin, move, tile ou drag, nem cancelar esses eventos pelas rotas excluídas. Esta exceção temporária não altera o toque nativo do compositor fora dessas rotas.

#### Scenario: Movimento por toque sobre a barra
- **WHEN** uma sequência de toque ocorre sobre título, ícone ou botão da decoração
- **THEN** a decoração não muda geometria, floating, pin ou snap e não cancela eventos por essa sequência

### Requirement: Cliques de mouse preservados e pareados na alpha-0
O sistema SHALL preservar botões, ícone, menu, roda, hover/tooltip, foco/elevação e duplo clique configurado. Cliques consumidos nos botões, ícone ou menu SHALL consumir press e release da mesma origem e botão, executando a ação uma única vez. Cliques não consumidos SHALL passar sem retenção por estado de drag/grab desativado. Mudança de foco ou ponteiro entre press e release MUST NOT produzir par órfão por intervenção da barra; release de outra origem/botão MUST NOT ser consumido como se fosse o par retido.

#### Scenario: Minimizar muda foco antes da soltura
- **WHEN** o botão minimizar consome press e o foco muda antes do release
- **THEN** a ação acontece uma vez e a soltura correspondente também é consumida
- **AND** a próxima interação não fica presa

#### Scenario: Ícone ou botão direito abre menu
- **WHEN** um clique no ícone ou com botão direito na barra abre o menu
- **THEN** press e release do clique são tratados em par mesmo se o ponteiro sair da barra
- **AND** nenhuma flag de drag/grab é armada

#### Scenario: Dois dispositivos ou barras sobrepostas
- **WHEN** há eventos intercalados de duas origens/botões ou duas barras recebem o mesmo press
- **THEN** somente uma ação ocorre para o clique consumido
- **AND** a soltura de outra origem/botão não é roubada

#### Scenario: Clique comum após grab indisponível
- **WHEN** uma tentativa de grab foi recusada pela alpha-0 e o usuário clica fora de uma ação da barra
- **THEN** o antigo estado de grab não consome press nem release

### Requirement: Menu sem comandos de grab mudos na alpha-0
Na alpha-0, o menu SHALL ocultar Mover e Redimensionar, preservando suas outras ações. Chamadas explícitas válidas à entrada Lua `grab` SHALL informar indisponibilidade sem mudar foco, iniciar/encerrar drag ou reter o próximo clique. Nomes e argumentos da entrada permanecem, e argumentos inválidos continuam diagnosticados.

#### Scenario: Abrir menu da janela
- **WHEN** o usuário abre o menu pela barra ou pela entrada de menu do núcleo
- **THEN** Mover/Redimensionar não são oferecidos
- **AND** minimizar, maximizar, fechar e demais ações preservadas continuam acessíveis

#### Scenario: Chamada explícita de grab
- **WHEN** alguém chama `hl.plugin.kwinish.grab` com modo e endereço válidos
- **THEN** recebe diagnóstico de indisponibilidade na alpha-0
- **AND** nenhum foco, aquisição, fim de drag ou cancelamento posterior é produzido pelo grab

### Requirement: Reativação do drag após alpha-0
O sistema MUST NOT reativar automaticamente drag da decoração/menu por uma opção runtime na alpha-0. A reativação SHALL pertencer à alpha-1 e depender de decisão explícita Q4/T10, recheck de `target()` em `KwDrag::begin` após o callback de decoração (T9) e autorização/validação próprias.

#### Scenario: Configuração de snap ou resistência habilitada
- **WHEN** snap preview, snap de borda ou resistência estão configurados durante a alpha-0
- **THEN** seus efeitos associados ao drag da barra permanecem inativos
- **AND** dados e opções persistidos não são removidos para desativar a funcionalidade
