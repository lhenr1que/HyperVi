# title-bar Specification

## Purpose

A barra de título que o plugin `kwinish-bars` põe nas janelas, no estilo do KWin: os botões, as ações deles e o retorno que dão ao passar o mouse.

## Requirements

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

### Requirement: Toque na decoração indisponível
Na alpha-0 e na alpha-1, o sistema MUST NOT interpretar down/move/up de toque na decoração do plugin como gesto, clique de ação ou pedido de float, resize, pin, move, tile ou drag, nem cancelar esses eventos. Esta exceção não altera o toque nativo do compositor fora dessas rotas.

#### Scenario: Movimento por toque sobre a barra
- **WHEN** uma sequência de toque ocorre sobre título, ícone ou botão da decoração
- **THEN** a decoração não muda geometria, floating, pin ou snap e não cancela eventos por essa sequência

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

### Requirement: Arrasto pela barra com fim pelo compositor
Na alpha-1 para Hyprland 0.57, o sistema SHALL mover a janela quando o usuário aperta o título e move o ponteiro além do limiar, iniciando o drag pelo controlador do compositor. As rotas da barra e do menu MUST NOT pedir fim, substituir ou reiniciar com fim um drag: o drag SHALL terminar pelo fim nativo do compositor, que recebe a soltura do botão sem cancelamento. A barra MUST NOT tomar press do botão esquerdo nem do direito enquanto houver drag ativo de qualquer origem. O estado de drag da barra SHALL ser armado só quando o início realmente ocorreu. A barra SHALL iniciar o drag só com a própria janela sob o ponteiro, pelo mesmo teste de janela que o início usa; com outra janela sob o ponteiro, a barra MUST NOT focar nem iniciar, e o press SHALL seguir como clique.

#### Scenario: Arrastar janela flutuante pelo título
- **WHEN** o usuário aperta o título vazio, move o ponteiro além do limiar e solta
- **THEN** a janela acompanha o ponteiro e o drag termina na soltura
- **AND** a soltura chega ao compositor e nenhum estado de drag da barra fica armado

#### Scenario: Drag de outro componente ativo
- **WHEN** um drag iniciado por outro componente (bind nativo, outra decoração) está ativo e o usuário aperta a barra com o botão esquerdo ou direito
- **THEN** a barra não toma o press, não abre menu e não inicia, substitui nem encerra esse drag

#### Scenario: Início recusado
- **WHEN** o compositor ou uma decoração recusa o início, ou já há drag ativo
- **THEN** a barra não arma estado de drag e o próximo clique não é retido

#### Scenario: Entrada capturada na soltura
- **WHEN** um cliente captura a entrada no momento da soltura e o compositor não encerra o drag
- **THEN** o drag da barra continua até a próxima tecla ou o próximo clique não capturado
- **AND** o plugin não pede fim para compensar (custo aceito pelo dono em 01/11/2026)

#### Scenario: Ponteiro sai da janela antes do início adiado
- **WHEN** o usuário aperta o título de uma janela lado a lado, maximizada ou encaixada e, no movimento que passa do limiar, o ponteiro já está sobre outra janela
- **THEN** nenhuma janela é arrastada, o foco não muda e nenhum estado de drag da barra fica armado
- **AND** a soltura é tratada como a de um clique

### Requirement: Encaixe, prévia e resistência ao arrastar pela barra
Durante o drag da barra, o sistema SHALL voltar a: encaixar ao soltar nas bordas e, com Shift, na zona sob o ponteiro, segundo `snap_top_edge` e o layout ativo; mostrar a prévia segundo `snap_preview` e `preview_color`; resistir ao cruzar a divisa entre monitores segundo `seam_resistance`; sair de maximizada ou encaixada pelo limiar de arrasto. O rearm por Shift SHALL só iniciar um drag novo, nunca encerrar antes, e só com a barra em drag, sem saída de encaixe pendente e com o ponteiro sobre a janela da barra. O encaixe ao soltar SHALL usar as ações explícitas existentes, inclusive o monitor do soltar. Se houver drag ativo de qualquer origem durante a saída de encaixe, a saída de maximizada ou o início adiado, a barra SHALL abortar: MUST NOT mudar mais a geometria nem o foco da janela, MUST NOT iniciar drag, e SHALL deixar a soltura desse press chegar ao compositor.

#### Scenario: Soltar na borda esquerda
- **WHEN** o usuário arrasta pela barra até a borda esquerda e solta
- **THEN** a prévia aparece durante o arrasto e a janela é encaixada à esquerda depois da soltura

#### Scenario: Arrastar janela maximizada
- **WHEN** o usuário arrasta pela barra uma janela maximizada além do limiar
- **THEN** a janela sai da maximização antes de o drag começar e acompanha o ponteiro

#### Scenario: Shift fora do drag da barra
- **WHEN** o usuário aperta Shift sem drag da barra ativo, ou durante um drag de outro componente
- **THEN** nada é rearmado nem encerrado pela barra

#### Scenario: Outro drag começa durante a saída de encaixe
- **WHEN** a janela está voltando ao tamanho anterior ao encaixe e outro componente inicia um drag
- **THEN** a barra para a animação onde está, não muda o foco e não inicia drag
- **AND** a soltura do botão chega ao compositor, que encerra o outro drag pelo fim nativo

### Requirement: Mover e Redimensionar pelo menu na alpha-1
O menu da janela SHALL voltar a oferecer Mover e Redimensionar. A entrada Lua `grab` SHALL iniciar o drag pelo controlador do compositor e só considerar o grab iniciado quando o início ocorreu; todo `grab` com argumentos válidos que não inicia drag SHALL ser diagnosticado como erro Lua, sem armar grab nem reter clique: janela não achada, oculta ou não mapeada, foco recusado e início recusado pelo compositor. O grab SHALL terminar pelo fim nativo do compositor e ser limpo em qualquer tecla; um clique de grab sem drag ativo SHALL passar.

#### Scenario: Mover pelo menu
- **WHEN** o usuário escolhe Mover no menu e move o ponteiro
- **THEN** a janela acompanha o ponteiro e o próximo clique termina o movimento pelo compositor

#### Scenario: Grab sem drag ativo
- **WHEN** o grab foi armado mas o drag já terminou (tecla, outro componente) e o usuário clica
- **THEN** o clique passa normalmente e nenhum estado de grab fica armado

#### Scenario: Grab que não começa
- **WHEN** `grab` é chamado com um endereço que não corresponde a janela mapeada e visível, ou o foco é recusado, ou o compositor recusa o início
- **THEN** a chamada termina com erro Lua que diz a causa e nenhum estado de grab fica armado

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

### Requirement: Símbolos respeitam a escala do monitor
O sistema SHALL renderizar os símbolos dos botões para a escala do monitor em que a barra está sendo desenhada.

#### Scenario: Barras em escalas diferentes
- **WHEN** uma textura de símbolo é reutilizada em outra escala
- **THEN** o símbolo é renderizado novamente para a escala atual
