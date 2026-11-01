# Q4-A — Posse do drag por sessão: contrato proposto e provas isoladas (30/10/2026)

Escopo autorizado: **somente** investigação, sondas e testes host-free. **Nenhum código de produção de Q4 foi escrito**; K3/K9/rearm, `PLUGIN_EXIT`, render/GL e ownership de decorações intocados; nada carregado; nenhum commit de Q4. Base: `f76c785` (P8-B). Legenda: `U:` = Hyprland 0.57.0 `0aa6745f` (= headers instalados); `HU:` = hyprutils **0.14.3** (instalado `0.14.3-1`; fonte `codeload.github.com/hyprwm/hyprutils/tar.gz/refs/tags/v0.14.3`, sha256 `87bbfc9549ed96316b0d592c16d01f58729b23a2999a5293b73eec620b40c5f5`, `Signal.hpp` idêntico ao instalado). **Q4 não está resolvido**: o que segue é contrato + simulação, sem execução no host.

**Estado final da continuação: BLOQUEADO para implementação.** O protótipo revisado reproduz duas falhas de R4 (fim externo pelo host e fim externo pelo plugin); GCC e Clang saem com 1, sem erro de sanitizer. O verde de 45+9 verificações e as 17 mutações descritos abaixo são intermediários, anteriores a R4. Nenhum parecer aprovou o contrato como base de implementação.

## 1. Fatos do host que o contrato usa (lidos nas fontes)

| # | Fato | Fonte |
|---|---|---|
| F1 | `m_target` (`WP<ITarget>`, privado) só é escrito em `dragBegin` (`=` :160) e `dragEnd` (`reset()` :265, :274) | `U:layout/supplementary/DragController.cpp` |
| F2 | `dragBegin` **não** verifica se já há drag; todos os chamadores do host verificam antes (`Window.cpp:1861,1871`, `InputManager.cpp:876`, `ConfigActions.cpp:1704`), assim como `KwDrag::begin` | idem; P8-B |
| F3 | `dragEnd` sem alvo vivo retorna `false` **sem** emitir `ended` (:253–256); com alvo, `ended` sai por *scope guard* na **saída** (:258) | idem |
| F4 | Recusas dentro de `dragBegin` chamam `dragEnd` (janela não mapeada :173–177, sem workspace :179–183, workspace com tela cheia em `updateDragWindow` :123–126) ⇒ `ended` emitido **durante** `beginDragTarget` | idem |
| F5 | Com janela não mapeada e alvo **sem** janela, `dragEnd` emite `ended` mas **não** zera `m_target` (:262–271) | idem |
| F6 | Antes de `ended`, `dragEnd` já zerou `m_target` (:274) e roda callbacks síncronos: decorações `INPUT_TYPE_DRAG_END` da janela sob o cursor (:287–293, com retorno antecipado), agrupamento, `setTargetGeom`, `fullWindowFocus` (:324 → ouvintes `window.active`, inclusive do plugin e do núcleo Lua) | idem |
| F7 | O host encerra drags sozinho: tecla (`keybinds/Manager.cpp:223`), botão salvo *input capture* (:353), `floatWindow` (`ConfigActions.cpp:252`), `Actions::mouse` (:1699), **unmap da janela arrastada** (`Window.cpp:1672–1673`, alvo ainda vivo ⇒ `ended`) | idem |
| F8 | `CSignalT::emit` copia os ouvintes vivos como `SP` antes de iterar; pula um ouvinte solto pelo dono **somente se** ficou uma única referência do snapshot (`strongRef()==1`); snapshots aninhados podem reter duas referências e permitir a chamada antiga; ouvinte criado durante a emissão não é chamado nela; o `SP` da cópia mantém o `CSignalListener` (e o `std::function` do plugin) vivo até o **fim** da emissão | `HU:src/signal/Signal.cpp:12–46` |
| F9 | O plugin não sobrescreve `onInputOnDeco` (`barDeco.hpp`); os únicos inícios de drag do plugin são `handleMovement` (ouvintes `mouse.button`/`mouse.move` da barra) e `luaGrab` (IPC/`hyprctl eval`); os `window.active` do núcleo (`core/kwinish.lua:234,334,769`) não chamam `grab` | leitura do código |

**Não há identificador de sessão** público (P8-B §2). `ended` não tem argumento: não diz **qual** sessão terminou.

## 2. Contrato proposto

Estado do plugin (por processo): `listener` (`CHyprSignalListener`, **por sessão**: existe só enquanto o plugin possui ou está iniciando um drag), `ownedAt` (geração na aquisição), `ownedTarget` (`WP<ITarget>`), `generation` (contador de `ended` aceitos), `acquisition` (identidade local do listener capturada no callback), `inBegin`, `inOwnEnd`, `unloading`.

- **C1 — Aquisição só por início efetivo.** `begin`: recusa se `unloading`, `inOwnEnd` ou `inBegin`; uma guarda RAII mantém `inBegin` durante toda aquisição, inclusive falha e callbacks; recusa (sem tocar) se `target()` existe. **Arma o ouvinte antes** de `beginDragTarget` (vê o `ended` de uma recusa interna, F4). Possui só se, depois da chamada: nenhuma `ended` ocorreu (`generation == G0`), `target()` não é nulo e é o alvo pedido. Senão desarma e devolve `REFUSED`.
- **C2 — Invalidação e isolamento de callbacks antigos.** Cada armação incrementa `acquisition` e captura esse token por valor. O callback ignora eventos se seu token não é o atual; caso contrário faz `++generation`, zera posse e solta a si mesmo. O desarme também incrementa `acquisition`. Não depender de `strongRef()==1`: callbacks antigos podem sobreviver em múltiplos snapshots. O token identifica uma aquisição **do plugin**, não a sessão do host. Um evento antigo entregue ao listener atual antes de começar a emissão ainda pode invalidar conservadoramente a posse atual.
- **C3 — Predicado candidato de posse** = ouvinte armado ∧ `ownedAt == generation` ∧ `ownedTarget` vivo ∧ `target() == ownedTarget`. **Insuficiente como prova de posse atual:** antes de `ended`, alvo igual pode já pertencer a terceiro (R4, §9).
- **C4 — Fim (K3/K9), proposta ainda bloqueada.** O protótipo chama `endDragTarget()` se C3 e marca `inOwnEnd`; sem C3 não faz nada. **Não integrar:** C3 pode aceitar sessão de terceiro durante fim reentrante; `inOwnEnd` só bloqueia início, não fim, no protótipo atual. Sem posse demonstrável, o plugin não pode encerrar a sessão por suposição.
- **C5 — Rearm** = C4 seguido de C1, no mesmo quadro; se C4 não encerrou (sem posse), não há rearm.
- **C6 — Falha ao armar** (`listen` lança): o modelo explora iniciar **sem** posse (preserva o início de hoje); o plugin nunca encerra esse drag. **Política ainda não aprovada:** isso não prova Q4 no unload, pois uma interação do plugin pode permanecer ativa sem posse verificável. Alternativa: recusar o início e fazer rollback das flags dos chamadores. O dono deve decidir antes de produção. Exceções dentro de `beginDragTarget`/callbacks do host não têm rollback provado por este modelo.
- **C7 — Unload passo 2:** `unloading = true`; se C3, C4; depois **sempre** solta o ouvinte (mesmo armado por um alvo morto em silêncio), antes de remover ouvintes/decorações/timers e de qualquer `dlclose`.

**Premissas (limites da API):** não reutilizar token local durante a vida de qualquer callback retido (o modelo usa `uint64_t`; wrap requer política antes de produção); A1 thread única (H-THREAD); A2 = F1 (verificado); A3 nenhum componente chama `beginDragTarget` com drag ativo (host verificado em F2; **plugins de terceiros: premissa**); A4 `ended` síncrono, uma por fim com alvo vivo (F3, verificado); A5 o unload não ocorre de dentro de uma emissão de `ended` (Q3/H-Q3, F8).

**Geração — conclusão verificada:** `ended` sem argumento só permite saber "alguma sessão terminou depois da aquisição". Com ouvinte **por sessão**, a geração é **redundante** (o próprio desarme já invalida; mutantes q09–q12 sobrevivem por isso), mas é ela que dá o retorno correto de `begin` na recusa sem reset de alvo (F5; mutante q13). Com ouvinte **permanente** (P7 §5), a geração é **essencial** (q14, q15). Proposta revisada: ouvinte por sessão + geração de eventos + token de aquisição capturado pelo callback + guarda de aquisição. Só a geração de eventos não isola aquisições reentrantes.

## 3. Matriz de transições (estado do plugin × evento)

| Estado | Evento | Ação do contrato | Resultado | Fonte / teste |
|---|---|---|---|---|
| sem posse, sem drag | `begin` válido | arma, `beginDragTarget`, verifica | **POSSUI** | S1 |
| sem posse, drag de outro | `begin` | recusa sem tocar | sem posse, drag de outro intacto | F2; S3 |
| sem posse | `begin` recusado dentro do host | `ended` interno → desarma; `REFUSED` | sem posse | F4/F5; S4.0–S4.3 |
| POSSUI | K3/K9 do plugin | `inOwnEnd`, `endDragTarget`, `ended` desarma | sem posse, sem drag | S1 |
| POSSUI | host encerra (tecla/botão/float/unmap) | `ended` desarma | sem posse | F7; S2 |
| (depois) | outro inicia drag na **mesma** janela; release do plugin | C3 falso ⇒ nada | drag do outro intacto | **contraexemplo S2** |
| POSSUI | alvo destruído sem `ended` | C3 falso (`target()` ≠ `ownedTarget`) | sem efeito; ouvinte ainda armado até C7 | F3; S5 |
| POSSUI | `dragEnd` por decoração (retorno antecipado) | `ended` desarma | sem posse | F6; S9 |
| POSSUI | `ended` sem reset de alvo | desarma | sem posse (conservador) | F5; S10 |
| durante `dragEnd` do host | callback de outro inicia drag | alvo distinto: C3 falso; mesmo alvo: C3 pode aceitar incorretamente | modo pode ser invalidado pelo host; **fim reentrante do plugin no mesmo alvo é inseguro** | F6; S7/R3 e R4 (§9) |
| durante C4 (`inOwnEnd`) | callback tenta `begin` do plugin | recusa | nenhuma sessão órfã | S8 |
| POSSUI | `begin` de terceiro **sem guarda** no mesmo alvo | indistinguível | **o plugin encerraria** | violação de A3 (teste documenta) |
| qualquer | unload | C7 | ouvinte solto; drag do outro intacto | S11–S13 |
| — | `listen` falha | C6 | drag iniciado, sem posse | S14 |

## 4. Provas host-free (`evidence/Q4A-*`)

Modelo (`Q4A-followup-tests/q4model.hpp`, suíte `q4test.cpp`, runner `run.sh` no mesmo diretório): réplica do fluxo de `dragBegin`/`dragEnd` 0.57 (linhas citadas em F1–F6) com o **sinal real** do hyprutils (`CSignalT<>`, `CHyprSignalListener`, `CScopeGuard`, biblioteca instalada). GCC 16.2.1 e Clang 23.1.4, `-fsanitize=address,undefined`, 0 avisos, saída idêntica: **45 OK, 0 falhas** na baseline (`Q4A-run.txt`). Revisão acrescenta **9 verificações** em `review_cases.hpp`; resultado final deve ser conferido no log `Q4A-followup-run.log`, sem contar o verde histórico como prova das regressões novas.

- **Contraexemplo reproduzido:** as três políticas sem posse (incondicional, igualdade de alvo, alvo+modo) encerram o drag do usuário na mesma janela (S2); a proposta e a variante permanente o mantêm.
- Proposta e variante permanente passam S1–S10 (S4 em 4 variantes); premissa A3 demonstrada como limite; semântica do hyprutils medida (autodesarme no próprio handler sem erro do ASan; ouvinte novo não chamado na emissão corrente; ouvinte solto antes da vez é pulado); unload (posse própria, drag de terceiros, alvo morto em silêncio, nenhum início após o passo 2); falha de `listen`.
- **Callback antigo e rearm:** um listener anterior inicia sessão nova dentro de `ended`; no caso simples com um snapshot, o callback antigo é pulado pelo sinal real. **Isso não vale para snapshots aninhados**: a regressão R2 reproduziu callback antigo ainda invocado; o token local passa a neutralizá-lo, mesmo quando ele roda. Rearm do drag próprio adquire sessão nova; se terceiro inicia durante o fim, o início seguinte recusa e não adota o drag de terceiro. Esses 4 checks foram acrescentados após montar o pacote da revisão.
- **Limite Q3 reproduzido:** se o módulo solta o ouvinte **de dentro** de uma emissão de `ended`, o estado do handler só é destruído no fim da emissão (depois do "unload") — o `std::function` do plugin roda o destrutor fora do controle do plugin. Exige A5.
- **Mutações do modelo** (`Q4A-mutations.log`): 15; **10 mortas** (guarda de drag ativo, `inOwnEnd`, `unloading`, desarme no handler, soltura no unload, política de falha, posse = "há alvo", geração na recusa, e as duas checagens de geração da variante permanente); **5 sobrevivem como redundância explicada** (geração e alvo vivo em C3, igualdade de alvo sob A3, ouvinte nulo = desarmado, verificação antes de C4 no unload). Duas lacunas reais apareceram (q05: unload com alvo morto em silêncio; q15: recusa sem reset de alvo) e ganharam teste.
- Um erro do **próprio teste** (callback que se apagava durante a execução) foi achado pelo ASan e corrigido no modelo (cópia antes de invocar), sem mudar a semântica.

**Limites:** réplica manual, não o `DragController` real; não executa `updateDragWindow`, foco, grupos, decorações reais, nem o laço do Hyprland; não prova A1, A3 (terceiros) nem A5; nenhum teste aninhado ou ao vivo. As sondas reais (`Q4A-followup-tests/probe.cpp`) passam GCC/Clang; negativo falha somente pelo membro `sessionId` ausente (não prova sozinho a ausência de qualquer API equivalente; essa conclusão vem da inspeção do header).

## 5. Reentrada e descarregamento

- **Reentrada no fim:** entre o reset do alvo (:274) e `ended` (:258/saída) rodam callbacks (F6). Um início reentrante de terceiro não recebe um **fim adicional** do plugin (S7); o `dragEnd` externo continua e zera seu modo para `INVALID`. Preservação de alvo não prova drag funcional. Esse clobber é do host e não foi corrigido neste escopo. Início reentrante do plugin: recusado em C4 por `inOwnEnd` (S8); durante um `dragEnd` **do host** o plugin não tem caminho de início conhecido (F9) — invariante a manter, não verificável pelo plugin.
- **Autodesarme** dentro do handler é seguro (F8, S-listener com ASan).
- **Ordem no unload (C7):** passo 2 antes de remover os demais ouvintes/decorações; o `endDragTarget()` do passo 2 é síncrono e roda os callbacks de F6 **com o módulo mapeado** (decorações, `window.active` do plugin) — o passo 2 não pode agendar trabalho diferido (tooltip, rearm, preview) com `unloading`. Ouvinte solto antes do fim de `PLUGIN_EXIT`.
- **Relação com Q1–Q5:** Q4 = "nenhuma interação dependente do módulo ativa, ou encerrada pelo passo 2" (`QUIESCENCE.md` §2): este contrato define **qual** drag o passo 2 pode encerrar (só o próprio) e reduz L17 (`LEDGER.md`) a "drag próprio encerrado; de terceiros intacto". Q2 (contador de entradas) deve contar o handler de `ended`; Q3/H-Q3 cobre A5 (unload fora de emissão de `ended`); Q5 inalterado. **Reconciliação proposta, ainda não aplicada aos contratos históricos:** o reset legado por flags em `QUIESCENCE.md` Q4 e `LEDGER.md` L17 não é critério executável na 0.57. Na futura revisão, separar (a) limpar flags locais sempre, (b) terminar host drag só com posse atual, (c) não emitir fim adicional quando a posse se perdeu, mesmo com flags velhas. Não aplicar literalmente os dois contratos conflitantes. O unload administrativo quiescente vem do IPC (fora de emissões, `QUIESCENCE.md` §1), compatível com A5; o caminho `reload()` também ocorre fora das emissões de config, mas **não** foi verificado contra emissões de `ended` (nenhuma `ended` é emitida por `reload()` nas fontes lidas — hipótese a confirmar).

## 6. Critérios para autorizar a implementação de Q4-A

1. Revisão independente deste contrato sem blocker/major.
2. Dono aceita as premissas A3 (terceiros) e A5 (unload fora de emissão de `ended`) como limites documentados, e a perda conservadora de posse (C2/F5).
3. **Não integrar listeners de produção sem autorizar e provar também sua soltura no lifecycle.** Estado C1–C3, K3/K9/rearm e C7 precisam de autorização explícita e coordenada; não basta implementar a posse e adiar a remoção do listener. Enquanto `PLUGIN_EXIT` permanecer fora do escopo, só protótipo host-free. Decidir C6 (falha de `listen`), política de exceções e rollback das flags antes da integração.
4. Antes do código: reproduzir vermelho→verde das regressões. Separar duas provas: (a) comparação com 0.56.2 só do comportamento histórico visível nos caminhos de drag próprio, respeitando diferenças deliberadas de segurança; (b) validação alinhada às fontes **0.57 fixadas** de begin/end, recusas internas, pontos síncronos e sinal real, com reentrada aninhada. Depois negativos de terceiros, mutações, sondas de tipos e comparação de erros. O modelo manual não substitui a segunda prova no host real.
5. Teste aninhado/ao vivo do dono antes de declarar Q4 resolvido: S2 real (atalho do usuário após tecla), unmap durante drag, rearm com Shift, unsnap, unload com drag ativo.

## 7. Tentativas iniciais de revisão (histórico, antes da continuação)

**Na primeira entrega: bloqueada; nenhum parecer independente obtido.** Tentativas em processos separados, somente pacote de fontes/contrato/testes, sem ferramentas de escrita:

| Provedor/modelo | Resultado real |
|---|---|
| Anthropic `claude-sonnet-4-6` | "Execution outcome is unknown"; sessão registrada, sem resposta de revisor recuperável; não reenviado à mesma sessão |
| Gemini `gemini-2.5-pro` | HTTP 404: modelo indisponível para novos usuários; API recomendou `gemini-3.1-pro-preview` |
| Gemini `gemini-3.1-pro-preview` | HTTP 429 `RESOURCE_EXHAUSTED`, quota free-tier 0 |

Provedor ChatGPT não utilizado nesta etapa, respeitando a indisponibilidade informada pelo dono. CLI de Claude/OpenCode/Codex não disponível no PATH; nada instalado, nenhuma configuração global alterada. Falhas preservadas em `evidence/Q4A-review-attempts.txt`. O pacote enviado está preservado como snapshot em `Q4A-review-prompt-1.txt`; os testes adicionados e as ressalvas posteriores **não foram revisados**. Resultado: material preparatório reproduzível, **não aprovado para implementação**; Q4 permanece aberto.


## 8. Continuação — revisão independente e reprodução (30/10/2026)

Revisão concluída em sessão nova Hermes / `openai-codex` / `gpt-6.1-sol`, com o contrato final e as fontes/testes reais, sem ferramentas de escrita. Anthropic/Gemini continuavam limitados; o pool OpenAI constava disponível e a chamada funcionou. Revisor da **mesma família do modelo da continuação**, não de outra família. Parecer `conditional`: **3 major, 2 minor**, todos avaliados sem mudança de produção.

- **R1 (major) reproduzido:** um `begin(A)` recusado emite `ended`; listener anterior chama `begin(B)` e adquire B; o pós-check externo desarma a posse de B. Mesmo alvo e alvo distinto testados. Correção no protótipo: `inBegin` com RAII recusa o início reentrante durante aquisição.
- **R2 (major) reproduzido:** `ended` externo → começo/fim de X → `ended` aninhado → aquisição B. O callback antigo tem duas referências de snapshot e roda, apagando B. Correção no protótipo: token por aquisição capturado, conferido **antes** de alterar geração/estado; desarme invalida token. Geração de eventos sozinha não basta.
- **R3 (major) reproduzido:** após início reentrante de X, alvo = X mas modo = `INVALID`; 8 falhas no teste vermelho incluindo as de R1/R2. Garantia reduzida a ausência de **fim adicional** pelo plugin; nenhuma compensação de modo ou mudança funcional do host foi implementada.
- **Minors:** critérios Q4/L17 históricos explicitamente pendentes de reconciliação; comparação histórica 0.56.2 separada da prova de fronteiras 0.57.

As 9 verificações novas passaram isoladamente GCC/Clang com ASan/UBSan. A validação intermediária completa passou: 45 verificações da baseline +9 regressões por compilador, sondas reais positivas/negativas, 17 mutações (12 mortas, 5 redundantes). Isso **não** representa o estado final após acrescentar R4 (§9). Nenhum resultado daqui autoriza implementação, altera C6 ou fecha Q4.

## 9. Bloqueio residual — janela antes da emissão de `ended` (R4)

A segunda revisão concluída (`evidence/Q4A-review-3.json`) deu **conditional**, com **1 major novo**. R1/R2 foram reconhecidos como corrigidos no protótipo; R3 foi reconhecido como limite corretamente reduzido. R4 foi reproduzido, **não corrigido**:

1. O plugin possui um drag na janela A.
2. O host começa `dragEnd()`, zera `m_target`, mas ainda não emitiu `ended`.
3. Num callback síncrono, um terceiro chama `dragBegin(A, MOVE)`; isso respeita A3 porque não havia alvo.
4. Ainda no callback, uma entrada de fim do plugin chama `endIfOurs()`.
5. Alvo e geração ainda casam com os antigos: C3 aceita, e o plugin **encerra o drag do terceiro**.

Acontece tanto se o fim externo veio do host quanto se veio do plugin. O token de callback não ajuda: o sinal ainda não foi emitido. No caso do fim próprio, consumir a posse antes da chamada e/ou uma guarda de fim reentrante pode fechar essa variante, mas **não** basta para o fim originado no host.

**Teste final:** `Q4A-followup-tests/review_cases.hpp` integra as duas regressões ao runner. GCC e Clang: **exit 1**, saídas idênticas, 45 checks anteriores e 9 regressões anteriores passam, **2 regressões R4 falham por assertiva**; nenhum erro ASan/UBSan ou aviso de compilação. Logs `Q4A-followup-final-red-{gcc,clang}.log`. Não recontar mutações numa baseline vermelha; o total 12/17 acima é do candidato intermediário.

**Gate adicional antes de implementar:** ou demonstrar estruturalmente que nenhuma entrada de fim do plugin pode ocorrer entre reset do alvo e `ended` em qualquer fim do host (incluindo callbacks de terceiros dentro do domínio aceito), ou obter um mecanismo de identificação/observação mais forte, com autorização própria se exigir hook/lifecycle. Essa restrição de reentrada **não** foi provada nem aceita pelo dono. Não acrescentá-la silenciosamente às premissas A1–A5.

Não há alegação de que toda solução seja impossível; o resultado é que **este contrato, sob as premissas declaradas, ainda é insuficiente**. Parada com o contraexemplo executável preservado, sem listeners de produção, sem alteração em K3/K9/rearm ou `PLUGIN_EXIT`, sem commit Q4.
