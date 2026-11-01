# title-bar Delta Specification

## ADDED Requirements

### Requirement: Arrasto pela barra com fim pelo compositor
Na alpha-1 para Hyprland 0.57, o sistema SHALL mover a janela quando o usuário aperta o título e move o ponteiro além do limiar, iniciando o drag pelo controlador do compositor. As rotas da barra e do menu MUST NOT pedir fim, substituir ou reiniciar com fim um drag: o drag SHALL terminar pelo fim nativo do compositor, que recebe a soltura do botão sem cancelamento. A barra MUST NOT tomar press do botão esquerdo nem do direito enquanto houver drag ativo de qualquer origem. O estado de drag da barra SHALL ser armado só quando o início realmente ocorreu.

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

### Requirement: Encaixe, prévia e resistência ao arrastar pela barra
Durante o drag da barra, o sistema SHALL voltar a: encaixar ao soltar nas bordas e, com Shift, na zona sob o ponteiro, segundo `snap_top_edge` e o layout ativo; mostrar a prévia segundo `snap_preview` e `preview_color`; resistir ao cruzar a divisa entre monitores segundo `seam_resistance`; sair de maximizada ou encaixada pelo limiar de arrasto. O rearm por Shift SHALL só iniciar um drag novo, nunca encerrar antes, e só com a barra em drag, sem saída de encaixe pendente e com o ponteiro sobre a janela da barra. O encaixe ao soltar SHALL usar as ações explícitas existentes, inclusive o monitor do soltar.

#### Scenario: Soltar na borda esquerda
- **WHEN** o usuário arrasta pela barra até a borda esquerda e solta
- **THEN** a prévia aparece durante o arrasto e a janela é encaixada à esquerda depois da soltura

#### Scenario: Arrastar janela maximizada
- **WHEN** o usuário arrasta pela barra uma janela maximizada além do limiar
- **THEN** a janela sai da maximização antes de o drag começar e acompanha o ponteiro

#### Scenario: Shift fora do drag da barra
- **WHEN** o usuário aperta Shift sem drag da barra ativo, ou durante um drag de outro componente
- **THEN** nada é rearmado nem encerrado pela barra

### Requirement: Mover e Redimensionar pelo menu na alpha-1
O menu da janela SHALL voltar a oferecer Mover e Redimensionar. A entrada Lua `grab` SHALL iniciar o drag pelo controlador do compositor e só armar o grab quando o início ocorreu; a recusa SHALL voltar a ser diagnosticada como na alpha-0, sem foco, aquisição ou retenção de clique. O grab SHALL terminar pelo fim nativo do compositor e ser limpo em qualquer tecla; um clique de grab sem drag ativo SHALL passar.

#### Scenario: Mover pelo menu
- **WHEN** o usuário escolhe Mover no menu e move o ponteiro
- **THEN** a janela acompanha o ponteiro e o próximo clique termina o movimento pelo compositor

#### Scenario: Grab sem drag ativo
- **WHEN** o grab foi armado mas o drag já terminou (tecla, outro componente) e o usuário clica
- **THEN** o clique passa normalmente e nenhum estado de grab fica armado

## REMOVED Requirements

### Requirement: Drag da decoração indisponível na alpha-0
**Reason**: A alpha-1 religa o drag com a política `alt5-rc` (decisão do dono em 01/11/2026).
**Migration**: Substituído por "Arrasto pela barra com fim pelo compositor" e "Encaixe, prévia e resistência ao arrastar pela barra".

### Requirement: Menu sem comandos de grab mudos na alpha-0
**Reason**: Mover e Redimensionar voltam ao menu.
**Migration**: Substituído por "Mover e Redimensionar pelo menu na alpha-1".

### Requirement: Reativação do drag após alpha-0
**Reason**: As condições foram cumpridas: decisão Q4/T10 (`dde3f03`), recheck T9 (`510d204`) e esta proposta com autorização própria.
**Migration**: Nenhuma; o requisito descrevia o caminho até a alpha-1.

## RENAMED Requirements

- FROM: `### Requirement: Toque na decoração indisponível na alpha-0`
- TO: `### Requirement: Toque na decoração indisponível`
- FROM: `### Requirement: Cliques de mouse preservados e pareados na alpha-0`
- TO: `### Requirement: Cliques de mouse pareados`

## MODIFIED Requirements

### Requirement: Toque na decoração indisponível
Na alpha-0 e na alpha-1, o sistema MUST NOT interpretar down/move/up de toque na decoração do plugin como gesto, clique de ação ou pedido de float, resize, pin, move, tile ou drag, nem cancelar esses eventos. Esta exceção não altera o toque nativo do compositor fora dessas rotas.

#### Scenario: Movimento por toque sobre a barra
- **WHEN** uma sequência de toque ocorre sobre título, ícone ou botão da decoração
- **THEN** a decoração não muda geometria, floating, pin ou snap e não cancela eventos por essa sequência

### Requirement: Cliques de mouse pareados
O sistema SHALL preservar botões, ícone, menu, roda, hover/tooltip, foco/elevação e duplo clique configurado. Cliques consumidos nos botões, ícone ou menu SHALL consumir press e release do mesmo botão, executando a ação uma única vez. O press no título que inicia drag é a exceção: sua soltura SHALL chegar ao compositor. Cliques não consumidos SHALL passar sem retenção. Mudança de foco ou ponteiro entre press e release MUST NOT produzir par órfão por intervenção da barra.

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
- **WHEN** o usuário aperta e solta o título sem passar do limiar
- **THEN** a janela é focada (e elevada, se flutuante) e nenhum drag é iniciado
