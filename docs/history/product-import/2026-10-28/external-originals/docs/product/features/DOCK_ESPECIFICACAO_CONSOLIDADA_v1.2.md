# Dock oficial — especificação funcional consolidada v1.2

**Produto:** interface padrão da futura suíte Arch; nome em aberto.  
**Data:** 28/10/2026  
**Estado:** design funcional. **Não implementado nem aprovado integralmente.**  
**Documento completo:** `ESPECIFICACAO_INTERFACE_SUITE_v1.2.md` (seção 18 e histórico D-01 a D-18).

## Decisões aprovadas para a dock

| ID | Decisão confirmada |
|---|---|
| D-02, D-11, D-12 | Rice oficial: **barra inferior e dock integrada** por padrão; acoplar/desacoplar é recurso do oficial, não obrigação de terceiros. |
| D-03 | **Ocultação inteligente**; gatilhos específicos ainda em discussão. |
| D-06 | Primeira Rice Store ativa **um rice completo por vez**, sem misturar componentes. |
| D-13 | Mostrar **favoritos globais + janelas do workspace ativo**. |
| D-14 | Floating, uma janela local: clique para **minimizar** se focada; **restaurar** se minimizada; focar se em segundo plano. |
| D-15 | Favorito aberto apenas em outro workspace: solicitar **nova janela no workspace atual**, sem transferir a remota. |
| D-16 | **Agrupamento por aplicativo ligado por padrão**, opção de desligar no Settings. |
| D-17 | **Pré-visualização híbrida configurável**: lista textual ou miniaturas, se suportadas; lista como fallback. |
| **D-18** | **Desagrupado = favoritos e tarefas separados**. Favorito é atalho permanente; cada janela local tem entrada própria. Repetição visual de ícones é permitida e intencional. |

## Dois estados do agrupamento

**Agrupado (padrão aprovado):** um ícone por aplicativo, abrangendo favorito e tarefas locais. Exemplo: navegador fixado + 3 janelas locais → 1 ícone de aplicativo com indicação de múltiplas janelas.

**Desagrupado (D-18 aprovada):** aplicativo favorito permanece na área de atalhos e cada janela aparece na área de tarefas. Exemplo: navegador fixado + 3 janelas locais → **1 favorito + 3 tarefas individuais**. Não há duplicação de janelas; há repetição visual intencional de símbolos com papéis diferentes.

## Interações propostas — ainda não aprovadas em detalhe

| Caso | Proposta |
|---|---|
| Favorito sem janelas locais | Abre ou solicita nova janela aqui, inclusive se aplicativo está ativo em outro workspace (D-15). |
| Favorito com 1 janela local | Mantém D-14 no Floating: minimizar/restaurar/focar; não vira botão obrigatório de “nova janela” quando o agrupamento é desativado. |
| Favorito com várias janelas locais | Abre seletor de janelas locais, sem minimizar todas. |
| Tarefa individual | Focar/restaurar ou minimizar **somente a janela selecionada** conforme estado e modo. |
| Clique central | Solicitar nova janela, se suportado. |
| Clique direito | Menu contextual conforme o tipo e estado da entrada. |
| Hover curto/prolongado | Nome/estado; preview textual ou miniatura quando disponível; nunca roubar foco. |
| Grupos | Mostrar seletor com janelas locais; fechar com Esc; revalidar identidade antes de agir. |

**Cuidado com o escopo:** informações de janelas em outros workspaces não aparecem como tarefas locais; podem ser mostradas como ações explícitas de navegação no menu do favorito. Janelas minimizadas devem permanecer associadas ao workspace **lógico** de origem.

## Menus contextuais propostos

- **Favorito sem janela local:** abrir aqui, nova janela quando possível, ir à janela remota (se existir), desafixar.
- **Favorito com janela local:** focar/restaurar ou minimizar, nova janela, desafixar, fechar **esta janela** se for inequívoca.
- **Favorito com 2+ janelas locais:** escolher janela, nova janela, desafixar; fechar uma janela somente após selecioná-la.
- **Tarefa individual:** focar/restaurar ou minimizar, fechar esta janela, fixar/desafixar o aplicativo.

**Exclusões recomendadas:** não incluir `Fechar todas` no menu principal. Menus devem informar ações indisponíveis e resultados reais, especialmente para apps de instância única.

## Visibilidade e modulação

**Decidido:** dock inteligente, integrada à barra inferior na instalação padrão. **Propostas ainda abertas:** manter o módulo integrado visível no Floating maximizado quando houver espaço reservado; recolher apenas os ícones em Tiling/Scroll sem esconder barra/status; ocultar completamente em fullscreen real; se dock desacoplada, recolher sob interseção e revelar por borda/atalho. Popovers sob interação devem impedir ocultação prematura.

## Priorização e testes

**Necessários para o produto:** seletor de múltiplas janelas com fallback textual, cliques corretos, favoritos por workspace, persistência de agrupamento, acesso teclado/ponteiro, proteção contra IDs obsoletos, ausência de duplicação de processos da dock. **Investigação posterior:** miniaturas reais em Wayland, latência e permissão de captura, multi-monitor em cenários complexos.

**Exemplos de aceitação:** (1) favorito + 3 janelas com agrupamento desligado mostra 4 entradas com papéis distintos; (2) agrupar/desagrupar não altera janelas; (3) clique em janela focada no Floating minimiza apenas ela; (4) favorito remoto pede janela local; (5) sem miniaturas, lista permanece funcional.

## Pendências conscientemente preservadas

1. Padrão exato da seleção entre miniaturas/lista: `Automático` foi recomendado, não aprovado.
2. Semântica exata de hover, MRU, tempo de popovers e menus.
3. Ocultação inteligente integrada em maximização, Tiling e Scroll.
4. Ordenação e drag-and-drop das tarefas; comportamento multimonitor.
5. Viabilidade real das miniaturas e integração com eventos Hyprland.

**Encaminhamento:** a dock está suficientemente consolidada em nível funcional para iniciar a especificação do **launcher**. Os cinco tópicos pendentes permanecem como propostas, sem serem apresentados como decisões já tomadas.
