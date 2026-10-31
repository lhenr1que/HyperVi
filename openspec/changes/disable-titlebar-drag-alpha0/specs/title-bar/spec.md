# title-bar Delta Specification

## ADDED Requirements

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
