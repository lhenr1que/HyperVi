# Launcher híbrido oficial — especificação funcional consolidada v1.0

**Produto:** rice oficial da futura suíte Arch Linux sobre Hyprland; nome definitivo do projeto em aberto.  
**Data:** 28/10/2026.  
**Natureza:** especificação de produto, não declaração de funcionalidades implementadas.  
**Referência geral:** `ESPECIFICACAO_INTERFACE_SUITE_v1.6.md`, seção 22; a documentação anterior do launcher (v0.1–v0.3) permanece como histórico de elaboração.  
**Escopo:** launcher do **rice oficial**. Não obriga os rices externos a adotarem seu launcher ou sua aparência. Não inclui dashboard, Canvas/VXWM inicial nem modularidade entre rices.

## 1. Convenções de decisão

- **APROVADO / D-19:** launcher híbrido; pesquisa imediata e navegação gráfica em uma única interface.
- **APROVADO / D-20:** tela inicial em layout **Split**; categorias à esquerda, grade de aplicativos à direita, busca no topo.
- **APROVADO / D-21:** pesquisa **global** de aplicativos e ações, sem restrição pela categoria selecionada.
- **APROVADO / D-22:** abertura **centralizada por padrão**, com **opção ancorada** na barra configurável no Settings.
- **BASELINE RECOMENDADA:** especificações operacionais que fecham um modelo funcional coerente e que ainda exigem aceite detalhado e verificação no código. Não são decisões tomadas pelo usuário, não são resultados de testes.
- **PENDENTE TÉCNICO / VISUAL:** detalhes dependentes de protótipo, integração com Hyprland, acessibilidade e identidade gráfica.

**Critério de encerramento documental:** o fluxo funcional está suficientemente especificado para planejar protótipos, contratos de API e testes. Aprovação para codificar, fidelidade visual e viabilidade de cada integração ainda não foram declaradas.

## 2. Princípios e fronteiras

1. **Buscar e iniciar:** o launcher encontra aplicativos e invoca ações explícitas. A dock mantém a responsabilidade de alternar, minimizar/restaurar e visualizar janelas abertas.
2. **Uma interface oficial por vez:** o launcher oficial não concorre com launcher de rice externo; um único rice completo fica ativo.
3. **Gerenciador independente:** Tiling, Scroll, Floating, workspaces e Settings fundamentais devem continuar disponíveis sem o launcher oficial.
4. **Pesquisa não é terminal:** digitar uma string não executa código/shell; Enter só ativa resultado selecionado do catálogo. Pesquisa de arquivos, web, clipboard e plugins são extensões futuras, não requisitos de MVP.
5. **Sem promessa de sucesso:** o sistema confirma aceitação/erro do pedido sem alegar criação/foco de uma janela não observada.
6. **Workspace contextual:** solicitações de abertura têm como alvo o workspace ativo no momento da **ativação** do resultado, conforme a capacidade real do Hyprland e do aplicativo.

## 3. Anatomia e estados de tela

| Região | Papel | Estado |
|---|---|---|
| Campo de busca superior | Entrada textual, cursor/foco inicial, botão de limpar opcional | **Híbrido e busca imediata aprovados**; detalhe de botão proposto |
| Coluna esquerda | Categorias dos `.desktop` e entrada separada `Ações` | **Categorias à esquerda aprovadas**; item Ações é baseline |
| Painel direito | Grade de aplicativos ao explorar; lista de resultados ao pesquisar; catálogo de ações ao escolher Ações | **Grade inicial aprovada**; apresentação durante busca e ações é baseline |
| Rodapé discreto | Ajuda de teclado e acesso ao Settings quando útil | Baseline; não duplicar status permanente da barra |

| Estado | Visualização recomendada | Evento de saída |
|---|---|---|
| **Fechado** | Nenhuma superfície do launcher | Botão na barra ou atalho |
| **Aberto sem busca** | Split; `Todos` selecionada; aplicativos em grade, alfabeticamente | Digitar / escolher categoria / ações |
| **Navegando categoria** | Split; grade filtrada pela categoria da esquerda | Digitar / escolher outra categoria |
| **Pesquisando** | Coluna esquerda mantém contexto; painel direito vira lista global `Aplicativo` / `Ação` | Apagar texto / selecionar resultado / Esc |
| **Ações** | Catálogo de ações no painel direito | Retornar categoria / pesquisa / Esc |
| **Confirmando operação sensível** | Confirmação modal com descrição da operação e alvo | Confirmar / cancelar |
| **Solicitação em curso** | Prevenção de acionamentos duplicados; indicação de processamento quando pertinente | Aceitação / erro / cancelamento |
| **Sem resultados** | Mensagem acessível, sem interpretar busca como comando | Editar/apagar termo |

**Baseline de navegação:** escolher categoria não executa aplicativos. Começar a digitar em qualquer categoria pesquisa globalmente; ao apagar a consulta, a grade retorna à categoria visual antes selecionada. Abrir novamente usa `Todos` e consulta vazia por padrão; persistir consulta/categoria entre aberturas fica para fase futura.

## 4. Posicionamento, entrada e monitores

### 4.1 Decisão D-22 (APROVADA)

- **Padrão inicial:** launcher centralizado no monitor de interação.
- **Alternativa no Settings:** launcher ancorado ao botão da barra oficial.
- **Mesma funcionalidade:** posição não altera pesquisa, categorias, ações nem os comandos disponíveis.

### 4.2 Regras derivadas recomendadas (BASELINE)

| Disparo e preferência | Posicionamento proposto |
|---|---|
| **Atalho de teclado** + preferência central | Central no monitor com foco do teclado |
| **Botão da barra** + preferência central | Central no monitor da barra de origem |
| **Botão da barra** + preferência ancorada | Junto ao botão, deslocando para dentro da área útil e sem ultrapassar bordas |
| **Atalho de teclado** + preferência ancorada | **Fallback central**, pois não existe botão de referência inequívoco |
| **Botão indisponível / barra oculta** + preferência ancorada | Fallback central |
| Troca de monitor ou escala fracionária | Recalcular geometria antes de mostrar; não recortar controles |

Preferência de posicionamento é específica do **rice oficial**. A área útil e a escala do monitor são respeitadas. Não há compromisso com tamanho, pixel exato ou animação antes do design system e testes. Se um monitor for desconectado com launcher aberto, manter uma interface acessível em monitor sobrevivente ou fechar com segurança sem deixar a sessão bloqueada.

**Fullscreen real:** invocação precisa ser explícita e testada; não abrir por hover. A política concreta de sobreposição do Hyprland e captura de foco é dependência técnica a validar.

## 5. Pesquisa global e apresentação dos resultados

**APROVADO:** qualquer termo é pesquisado globalmente no catálogo de aplicativos e ações disponíveis, mesmo se a coluna esquerda mostrar `Internet` ou outra categoria.

**BASELINE:**

- Indexar aplicativos instalados via metadados `.desktop` aplicáveis ao ambiente (nome traduzido, palavras-chave, descrição, categoria, ícone, Exec/ações disponibilizadas), respeitando itens ocultos/indisponíveis na sessão.
- Indexar ações da suíte explicitamente registradas; **não** interpretar a busca como comando shell.
- Normalizar maiúsculas/minúsculas e, quando possível, acentos; suporte a IME e acessibilidade. Matching de prefixo e nome preferido a descrição; ordenação estável e previsível para empates.
- Resultados exibem ícone, nome, tipo (`Aplicativo` ou `Ação`) e descrição concisa. Ações destrutivas devem ser claramente sinalizadas, mas sem destaque de cor indevido.
- Limitar consumo de recursos; indexação assíncrona quando necessária; evitar congelar UI em catálogos grandes. Cache e atualização por mudança de `.desktop` são opções de implementação, não API exigida.
- Não armazenar histórico de termos, telemetria de consulta ou frequência de uso por padrão. Uma futura seção de recentes dependerá de escolha explícita e controles de privacidade.

**Exemplo obrigatório:** selecionar `Internet`, digitar `terminal` e obter Terminal (caso instalado e pesquisável), sem trocar a categoria visual.

## 6. Catálogo de ações rápidas

**BASELINE funcional recomendado**, a confirmar na integração. Disponibilizar por item `Ações` na coluna esquerda e pela busca global.

| Grupo | Ação | Contexto / proteção |
|---|---|---|
| Janelas | **Aplicar Tiling** | Workspace ativo, via API do gerenciador; indicar modo atual |
| Janelas | **Aplicar Scroll** | Mesmo contexto |
| Janelas | **Aplicar Floating** | Mesmo contexto |
| Configurações | **Abrir Settings** | Aplicativo fundamental, sem depender da interface do rice |
| Configurações | **Configurar monitores** | Abrir página/serviço correspondente se disponível |
| Sessão | **Bloquear tela** | Serviço configurado; feedback de indisponibilidade |
| Sessão/energia | **Encerrar sessão, Reiniciar, Desligar** | Exigir confirmação contextual explícita e integração confiável |

A ação de modo não afeta outros workspaces nem promete alterar um modo não suportado. Expor “estado atual” somente com leitura confiável da API. VXWM é futuro e não aparece como funcionalidade inicial. Ações indisponíveis recebem estado desativado ou explicação verificável, jamais sucesso fictício.

**MVP recomendado:** apps, três modos, Settings, monitores e bloquear; energia sensível pode entrar depois de integração testada. A presença destas ações é recomendação, não um novo requisito aprovado pela escolha D-22.

## 7. Teclado, mouse e foco

**BASELINE recomendado:**

| Entrada | Comportamento |
|---|---|
| Abrir por botão na barra | Abre no monitor do botão, com campo de busca focado |
| Atalho global configurável | Abre no monitor com foco; repetição com launcher aberto alterna abrir/fechar; não duplica superfícies |
| Digitar | Insere no campo imediatamente e pesquisa globalmente; nenhuma execução implícita |
| `↑` / `↓` | Quando há resultados, percorre seleção; em grade, navega verticalmente ao assumir foco dos itens |
| `←` / `→` | Dentro da busca, move cursor textual normalmente; na grade focada, muda de coluna |
| `Tab` / `Shift+Tab` | Alterna de modo acessível entre busca, categorias, grade/resultados e controles |
| `Enter` | Ativa apenas resultado/controle selecionado; não executa consulta livre sem resultado identificado |
| `Espaço` | Digita espaço no campo; em botões/itens focados usa semântica da UI |
| `Esc` sem confirmação | Fecha o launcher sem executar ação; limpar busca é controle explícito separado |
| `Esc` em confirmação | Cancela **apenas a confirmação** e retorna ao launcher |
| Clique em categoria | Atualiza grade sem executar nada |
| Clique em app/ação | Solicita execução selecionada; feedback conforme retorno |
| Clique fora | Fecha launcher quando não houver confirmação modal |

Atalho sugerido para avaliação: `Super + Espaço`, apenas após auditar conflitos com bindings existentes. `Super` isolada não é decisão. Todos os controles exigem foco visível, nomes acessíveis, navegação por teclado e suporte a composição de caracteres. Hover não troca foco de janela nem executa resultado. Seleção ativa deve ser visualmente distinta sem depender só de cor.

## 8. Abertura de aplicativos, execução e fechamento

**BASELINE recomendada, com dependências técnicas:**

1. O usuário aciona aplicativo identificado; o launcher registra o **workspace e o monitor ativos naquele instante**, não ao abrir o launcher.
2. Encaminha a solicitação por meio confiável de lançamento (`.desktop` / serviço de aplicação), sem construir comandos shell usando a consulta pesquisada.
3. O gerenciador tenta estabelecer o novo contexto no workspace de destino, quando suportado. Não afirmar que há controle absoluto sobre instâncias gerenciadas por aplicativos.
4. Se o aplicativo reutilizar **janela single-instance** em outro workspace, não deslocar a janela silenciosamente. Oferecer acesso explícito à existente quando seu estado for observável.
5. Para pedido aceito, o launcher pode fechar; **pedido aceito não equivale a janela iniciada com sucesso**. Falhas posteriores precisam permanecer visíveis ou recuperáveis (notificação/estado), sem falso sucesso.
6. Falha imediata mantém contexto de busca e informa erro. Evitar envio duplicado por cliques ou Enter repetidos; não impor bloqueio permanente por timeout.
7. Abrir via launcher **não minimiza** uma janela já focada: a alternância minimizar/restaurar é regra da **dock no Floating**, não do launcher.
8. Em ação sensível, mostrar confirmação; `Esc` cancela a confirmação. Atualizar ícone/estado do modo somente após retorno real.

**Instância de UI:** uma superfície lógica de launcher oficial por sessão. Se o rice for trocado, o launcher oficial deve ser encerrado e não disputar foco com o do rice externo.

## 9. Settings — opções do launcher

**Aprovada:** `Posição do launcher`: `Centralizado` (padrão) / `Ancorado à barra`. A ancoragem com atalho usa fallback central (baseline).

**Outras opções propostas:** atalho global, comportamento de toque/segundo acionamento, dimensões adaptativas, preferências de categorias, redução de animação, acessibilidade, eventualmente ordenação/recência. Não criar configurações sem efeito ou controles de rices externos sem contrato de integração.

## 10. Compatibilidade, qualidade e limites

- **Wayland/Hyprland:** investigar métodos seguros de posicionamento, teclado, layer-shell/top-level e foco; não assumir que GTK4 oferece tudo sem adaptação. O toolkit definitivo não está aprovado.
- **API do gerenciador:** comandos de Tiling/Scroll/Floating e consulta de estado devem usar interface estável; comportamentos de falha, timeout e corrida precisam de testes.
- **Monitores:** testar escalas diferentes, monitor removido, multimonitor, botão em barra de monitor secundário e troca de workspace na hora de ativar.
- **Acessibilidade:** teclado completo, leitor de tela onde aplicável, contraste, seleção, composição/IME, foco com popup e fechamento sem perda indevida.
- **Privacidade:** sem histórico por padrão, miniaturas de janelas são competência da dock, não requisito do launcher.
- **Escopo excluído do MVP:** dashboard, busca universal de arquivos/web, clipboard, calculadora e plugins, execução arbitrária de comandos, Canvas/VXWM, combinação modular de rices.
- **Não implementado/testado por este documento:** nenhuma alteração no código foi feita nesta etapa.

## 11. Critérios de aceitação candidatos (NÃO EXECUTADOS)

| ID | Cenário | Resultado esperado |
|---|---|---|
| LH-01 | Acionar launcher pela primeira vez | Abre **centralizado** no monitor de interação; Split e busca focada |
| LH-02 | Alterar Settings para `Ancorado` e clicar no botão da barra | Abre junto ao botão, sem extrapolar área útil |
| LH-03 | Settings `Ancorado` e abrir por atalho | Fallback **central** coerente, sem tentar inventar coordenada de botão |
| LH-04 | Selecionar Internet, pesquisar `terminal` | Terminal aparece em busca global sem troca de categoria |
| LH-05 | Apagar busca após estar na categoria Desenvolvimento | Retorna à grade de Desenvolvimento |
| LH-06 | Navegar só por teclado | Busca/categorias/grade/ações com foco explícito e Enter seguro |
| LH-07 | Digitar texto parecido com comando shell | Nunca executa texto livre automaticamente |
| LH-08 | Acionar Reiniciar e pressionar Esc | Cancela confirmação; nada é reiniciado |
| LH-09 | Solicitar Floating | Atinge apenas workspace alvo; reporta retorno real |
| LH-10 | Iniciar app single-instance já aberto em outro workspace | Não muda janela remota silenciosamente; feedback honesto |
| LH-11 | Repetir atalho em sequência | Não empilha múltiplas superfícies |
| LH-12 | Dois monitores com escala diferente e botão em secundário | Posição e tamanho adaptados à área útil correta |
| LH-13 | Launcher aberto enquanto monitor é removido | Não deixa UI inacessível nem foco preso |
| LH-14 | Iniciar aplicativo via launcher enquanto janela dele já está focada | Não minimiza por herança da dock |
| LH-15 | Ativar rice externo completo | Launcher oficial não permanece como segunda interface sobreposta |
| LH-16 | Serviço de ação está indisponível | Controle informa indisponibilidade/erro, não sucesso |
| LH-17 | Aplicativo retorna aceitação sem janela observável | UI não declara que uma janela foi criada; falha subsequente recuperável |
| LH-18 | Abrir launcher em sessão fullscreen | Não aparece acidentalmente; invocação explícita obedece política testada |

## 12. Decisões restantes — não bloqueiam wireframes nem Settings

1. **Preset do atalho global**: candidato `Super + Espaço`; depende de levantamento de colisões com bindings existentes.
2. **Ordenação detalhada**: base alfabética, relevância em busca; empates e prioridades configuráveis a validar com protótipo.
3. **Catálogo definitivo de Ações**: especialmente operações de energia e serviços disponíveis.
4. **Padrão de encerramento após ativação**: baseline de fechar após aceitação; avaliar testes de usabilidade e erros tardios.
5. **Dimensões, animações, radius, cores, blur, responsividade**: parte do design system e mockups.
6. **Toolkit e contratos de API**: decisões de engenharia posteriores, sem reescrever o núcleo existente por conveniência estética.

**Marco:** especificação funcional do launcher **consolidada para a etapa de concepção visual e planejamento técnico**. As quatro decisões D-19–D-22 estão aprovadas. As demais regras são a **baseline recomendada** de implementação e teste, não aprovações presumidas do usuário.
