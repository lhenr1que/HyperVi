# Matriz de testes — Mostrar área de trabalho e restauração D-43

**Versão:** 1.2 · **Data:** 28/10/2026 · **Status:** critérios planejados, não executados.  
**Objetivo:** orientar testes sobre o núcleo Lua/Hyprland existente, preservando D-36 a D-44.

| ID | Pré-condição | Evento | Resultado exigido | Tipo |
|---|---|---|---|---|
| `SD-D43-01` | A2 revelado, B5 normal | A alterna A2→A3 | Restaura somente A2; B5 intacto; retornar A2 normal | Integração |
| `SD-D43-02` | A2 revelado | Foco vai para monitor B sem mudar A2 | A2 continua revelado | Integração |
| `SD-D43-03` | A2 revelado | Duplo evento A2→A3 | Restore idempotente, sem toggle extra | Unitário/eventos |
| `SD-D43-04` | A2 com duas janelas visíveis e uma já minimizada | Revelar, trocar A3 e voltar | Duas visíveis; a terceira segue minimizada | E2E |
| `SD-D43-05` | A2 Floating com geometrias distintas | Revelar, alternar A3 e voltar | Posições, empilhamento onde suportado, marcas e dimensões preservadas | E2E |
| `SD-D43-06` | A2 Tiling | Revelar, alternar, retornar | Modo e distribuição intactos | E2E |
| `SD-D43-07` | A2 Scroll com cliente fora do viewport | Revelar, alternar, retornar | Ordem/posição do Scroll preservadas; todas janelas acessíveis | E2E |
| `SD-D43-08` | A2 revelado | Janela escondida fecha | Não ressuscitar nem operar endereço reaproveitado | Unitário/eventos |
| `SD-D43-09` | A2 revelado | Janela muda de workspace manualmente | Não sequestrar janela nem ignorar conflito | Integração |
| `SD-D43-10` | A2 revelado | Abrir Visão Geral e explorar B5; Esc | Sem ativação de B5 e sem restore por inspeção | E2E |
| `SD-D43-11` | A2 revelado | Reload do núcleo Lua | Estado reconciliado; nenhuma janela presa no especial | Resiliência |
| `SD-D43-12` | A2 revelado | Reiniciar compositor / sessão nova | Não aplicar snapshot de sessão anterior a janelas novas | Resiliência |
| `SD-D43-13` | A2 revelado | Remover monitor A | Recuperar janelas sem operar monitor inexistente | Resiliência |
| `SD-D43-14` | A2 revelado | Comando de restauração falha parcialmente | Estado mantém pendências; erro/diagnóstico; novo retry seguro | Falha |
| `SD-D43-15` | A2 revelado | Timer/retorno atrasado após restore completo | Nenhuma nova ocultação ou deslocamento | Corrida |
| `SD-D43-16` | A2 revelado | Movimento rápido A2→A3→A2 | Sem reentrada em estado revelado nem duplicidade | Corrida |
| `SD-D43-17` | A2 revelado | Desconectar/reconectar monitor com ID distinto | Verificação de IDs; sem trocar workspace errado | Multimonitor |
| `SD-D43-18` | A2 Floating | Minimizar/restaurar uma janela antes e depois do fluxo | Marca do Floating preservada, regressão do bug conhecido | Regressão |
| `SD-D43-19` | Outro workspace/monitor tem apps fullscreen | Revelar/sair de A2 | Não afetar fullscreen remoto; proteção de sessão intacta | Segurança |
| `SD-D43-20` | A2 revelado, uma janela aberta na dock | A2→A3 | Dock volta ao estado real após restore, sem falsos minimizados | UI |

## Dados de teste e oráculos

- Mock/fake backend deve emitir **eventos reais de transição e foco**, não apenas atualizar campos internos; os dispatches simulados que não disparam eventos foram identificados como lacuna na auditoria anterior.
- Validar o estado antes/depois via consultas do compositor e estado do núcleo, sem inferir sucesso apenas de comandos aceitos.
- Injetar evento duplicado, atraso, erro de IPC, fechamento de janela e reuse de endereço.
- Repetir nos modos Tiling, Scroll e Floating, em um e dois monitores.
- Comparar snapshot antes/depois das janelas **elegíveis**; não comparar pixels das imagens geradas.
- D-44 **já aprovou** o tratamento de nova janela no workspace revelado; implementar e testar após auditoria. O caso de reutilizar janela existente sem criação permanece em aberto (P-17.2c).

## Critérios de pronto

1. `SD-D43-01` a `SD-D43-20` implementados ou com bloqueios explicitamente rastreados; riscos altos não podem ser marcados como concluídos por testes isolados.
2. Testes anteriores de minimização, modos e Snap Assist permanecem passando.
3. Sem comandos destrutivos a janelas de IDs obsoletos e sem estado escondido persistente após reload.
4. Eventos de recuperação, pendências e falhas têm diagnóstico verificável.
5. Nada é considerado implementado com base apenas em mockup ou documentação.


---

## Novos testes D-44 — criação de janelas e restauração

**Status:** aprovados como requisitos de produto, mas **ainda não executados**. Não presumir que a API atual possui callbacks ou IDs estáveis.

| ID | Pré-condição | Evento | Resultado obrigatório | Tipo |
|---|---|---|---|---|
| `SD-D44-01` | A2 Floating revelado, Firefox e Terminal ocultos | Abrir Arquivos efetivamente em A2 | Firefox/Terminal restaurados, Arquivos visível, posições anteriores preservadas quando aplicável | E2E |
| `SD-D44-02` | A2 revelado e uma janela já minimizada antes | Criar nova janela elegível em A2 | Janela previamente minimizada continua minimizada | Regressão |
| `SD-D44-03` | A2 revelado, B5 normal | Abrir janela em B5 | A2 permanece revelado; B5 segue independente | Multimonitor |
| `SD-D44-04` | A2 revelado | Spawn sem mapeamento / programa falha | A2 não restaura por D-44 | Unitário |
| `SD-D44-05` | A2 revelado | Notificação ou popup não elegível | Não dispara restore de A2 | Integração |
| `SD-D44-06` | A2 revelado | Nova janela e mudança A2→A3 com eventos concorrentes | Restauração única por geração, sem toggle adicional | Corrida |
| `SD-D44-07` | A2 Tiling revelado | Abrir nova janela em A2 | Modo Tiling preservado; novas janelas participam do layout normal | E2E |
| `SD-D44-08` | A2 Scroll revelado e janela fora do viewport | Abrir janela elegível em A2 | Todas as janelas acessíveis, ordem/estado do Scroll consistentes | E2E |
| `SD-D44-09` | A2 revelado | Janela mapeia e fecha antes da reconciliação | Não atinge identidade reutilizada nem deixa ocultação irrecuperável | Corrida |
| `SD-D44-10` | A2 revelado | Aplicativo de instância única reutiliza janela existente | Não confundir com criação; registrar decisão pendente P-17.2c | Limite contratual |
| `SD-D44-11` | A2 revelado | Reload durante restore disparado por nova janela | Recuperação rastreável e janelas não ocultas permanentemente | Resiliência |
| `SD-D44-12` | A2 revelado | Nova janela numa sessão/monitor identificado de forma obsoleta | Revalidar e impedir restore com destino antigo | Resiliência |

### Critério adicional de aceite

Não declarar D-44 implementada enquanto existirem casos de restauração parcial sem sinalização, interferência com outro monitor, reativação de janela previamente minimizada ou falha de recuperação após reload. Validar a **ocorrência efetiva de novo toplevel elegível no escopo**, não a intenção de iniciar programa. Preservar testes D-43 sem regressões.

---

## Novos testes D-45 — restaurar e focar janela ocultada ao clicar na dock

**Aprovados como requisitos de comportamento; não executados.** Usar o mesmo backend real de Show Desktop/miniaturização, sem nova implementação concorrente.

| ID | Pré-condição | Evento | Resultado obrigatório | Tipo |
|---|---|---|---|---|
| `SD-D45-01` | A2 Floating revelado; Firefox e Terminal ocultos | Clique explícito em Firefox na dock | Restaurar Firefox e Terminal; focar Firefox; NORMAL | E2E |
| `SD-D45-02` | A2 revelado; Música já minimizada antes | Clique em Firefox oculto | Música permanece minimizada; demais janelas antes visíveis restauradas | Regressão |
| `SD-D45-03` | A2 Floating revelado; Firefox era a última janela focada | Clique em Firefox | Restaurar e focar; **não minimizar** por regra de click-toggle Floating | Regressão |
| `SD-D45-04` | A2 revelado, B5 com janelas visíveis | Clique em Terminal de A2 | B5 permanece intacto, foco final em Terminal de A2 | Multimonitor |
| `SD-D45-05` | A2 revelado; hover de ícone ou prévia | Hover sem clique | Nenhuma restauração, foco ou minimização | UI |
| `SD-D45-06` | A2 revelado; janela oculta fecha antes de clique | Clicar entrada obsoleta | Não focar endereço reutilizado nem outra janela por engano; recuperar entradas restantes | Corrida |
| `SD-D45-07` | A2 revelado; Firefox oculto | Clique coincide com saída A2→A3 | Uma única restauração; foco não volta indevidamente a A2 após troca | Corrida |
| `SD-D45-08` | A2 revelado; Firefox oculto | Clique coincide com nova janela em A2 | Uma única restauração D-44/D-45; foco conforme intenção vigente e backend | Corrida |
| `SD-D45-09` | A2 Tiling revelado | Clique em cliente oculto | Restaurar conjunto sem mudar layout; focar cliente selecionado | E2E |
| `SD-D45-10` | A2 Scroll com janela fora do viewport | Clicar sua entrada de dock quando elegível | Restaurar e focar sem perda da janela nem corrupção do Scroll | E2E |
| `SD-D45-11` | A2 revelado; aplicativo favorito só aberto em B5 | Clique no favorito | Aplicar política existente de **nova janela em A2**; não restaurar A2 diretamente por D-45 | Regra cruzada |
| `SD-D45-12` | A2 revelado; cliente temporariamente oculto | Restore concluído, comando de foco falha | Estado normal preservado, erro de foco informado, sem reocultação | Falha |
| `SD-D45-13` | A2 revelado; cliente oculto | Reload ou hotplug entre clique e foco | Revalidar sessão/monitor/workspace/cliente; sem foco em alvo inválido | Resiliência |
| `SD-D45-14` | A2 revelado; abrir overview e inspecionar cliente | Inspeção sem selecionar/focar | Não disparar D-45; sem restauração involuntária | Integração |

**Oráculos:** observar o estado efetivo do compositor, a proveniência por janela e o resultado confirmado da operação de foco. Não usar apenas estado visual da dock, mockups ou retorno otimista de API como comprovação. A aceitação inclui todos os testes anteriores D-43 e D-44 e a correção de janelas inacessíveis após reload.
