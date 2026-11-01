# title-bar Delta Specification

## MODIFIED Requirements

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
