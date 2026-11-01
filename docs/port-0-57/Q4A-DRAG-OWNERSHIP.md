# Q4-A — Posse do drag por sessão: contrato proposto e provas isoladas (30/10/2026)

> **Atualização mais recente — Q4-A1, alternativa 5 (estudo host-free; Q4-A continua BLOQUEADA).** Ver [evidence/Q4A1-alt5-contract.md](evidence/Q4A1-alt5-contract.md). A alternativa 5 (o Kwinish não pede fim sem posse demonstrável e deixa o host encerrar o drag com a soltura **não cancelada**) com recheck de `target()` no helper (`alt5-rc`) passa, no modelo, O1 (nenhum fim de terceiro por pedido do Kwinish), O3 (entrada não bloqueada) e O4 (estado final) em 39 cenários. Mas **falha liveness (O2)** sob captura de entrada (C1/C2, regressão frente ao atual) e com teclas que não chegam ao fim nativo (K2/K3, preexistente). Exige mudar rotas de input (não autorizado). Os `floatWindow` do núcleo Lua continuam encerrando qualquer drag. Runner `python3 docs/port-0-57/evidence/Q4A1-tests/run.py`: exit 0, 15/15 mutações, estudo anterior preservado. Revisão `gpt-6.1-sol`: `conditional` → `conditional` (achados reproduzidos/tratados; bloqueio julgado honesto).

> **Atualização mais recente — política de encerramento (estudo host-free; implementação continua BLOQUEADA).** Ver [evidence/Q4A-policy-contract.md](evidence/Q4A-policy-contract.md). Candidata `veto-rc`: posse verificável (início próprio com resultado `STARTED`, ouvinte por aquisição com token, alvo vivo), **veto quando `wasDraggingWindow()`** (janela de callbacks do `dragEnd` mapeado), posse consumida antes do fim, fim/início não reentrantes e **recheck de `target()` no helper após `DRAG_START`** (mudança no helper P8-B, não autorizada). Runner `python3 docs/port-0-57/evidence/Q4A-policy-tests/run.py`: 42 cenários × 7 políticas, GCC/Clang ASan/UBSan, matriz reproduzida, 11/16 mutações detectadas. `veto-rc` ainda falha `R4.emit.host` (ouvinte de `ended` de terceiros antes do Kwinish: premissa D1, não aceita) e o limite A3 (`L1`). Revisão `gpt-6.1-sol`: `conditional` (1 major reproduzido e tratado como pré-condição A3′) → `approve_as_planning_basis` (só minors). R4 antigo preservado: `Q4A-followup-tests/run.sh` continua exit 1.

> **Atualização da continuação: revisão concluída, implementação BLOQUEADA.** O texto abaixo preserva a proposta e os testes da primeira entrega, não o veredito atual. Duas revisões independentes deram `conditional`; a segunda identificou R4: o alvo pode voltar a pertencer a terceiro antes de `ended`, enquanto a posse antiga ainda parece válida. O contrato revisado e a reprodução estão em [evidence/Q4A-followup-contract.md](evidence/Q4A-followup-contract.md), especialmente §9. **Estado final GCC/Clang: exit 1, duas regressões R4 falhando por assertiva.** Não usar o verde histórico de 45 checks como aprovação. Runner atual: `bash docs/port-0-57/evidence/Q4A-followup-tests/run.sh`; ele deve falhar enquanto R4 estiver aberto. Nenhuma produção ou commit Q4 autorizado.

Escopo autorizado: **somente** investigação, sondas e testes host-free. **Nenhum código de produção de Q4 foi escrito**; K3/K9/rearm, `PLUGIN_EXIT`, render/GL e ownership de decorações intocados; nada carregado; nenhum commit de Q4. Base: `f76c785` (P8-B). Legenda: `U:` = Hyprland 0.57.0 `0aa6745f` (= headers instalados); `HU:` = hyprutils **0.14.3** (instalado `0.14.3-1`; fonte `codeload.github.com/hyprwm/hyprutils/tar.gz/refs/tags/v0.14.3`, sha256 `87bbfc9549ed96316b0d592c16d01f58729b23a2999a5293b73eec620b40c5f5`, `Signal.hpp` idêntico ao instalado). **Q4 não está resolvido**: o que segue é contrato + simulação, sem execução no host.

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
| F8 | `CSignalT::emit` copia os ouvintes vivos como `SP` antes de iterar; pula os que o dono soltou antes da vez (`strongRef()==1`); ouvinte criado durante a emissão não é chamado nela; o `SP` da cópia mantém o `CSignalListener` (e o `std::function` do plugin) vivo até o **fim** da emissão | `HU:src/signal/Signal.cpp:12–46` |
| F9 | O plugin não sobrescreve `onInputOnDeco` (`barDeco.hpp`); os únicos inícios de drag do plugin são `handleMovement` (ouvintes `mouse.button`/`mouse.move` da barra) e `luaGrab` (IPC/`hyprctl eval`); os `window.active` do núcleo (`core/kwinish.lua:234,334,769`) não chamam `grab` | leitura do código |

**Não há identificador de sessão** público (P8-B §2). `ended` não tem argumento: não diz **qual** sessão terminou.

## 2. Contrato proposto

Estado do plugin (por processo): `listener` (`CHyprSignalListener`, **por sessão**: existe só enquanto o plugin possui ou está iniciando um drag), `ownedAt` (geração na aquisição), `ownedTarget` (`WP<ITarget>`), `generation` (contador de `ended` vistos), `inOwnEnd`, `unloading`.

- **C1 — Aquisição só por início efetivo.** `begin`: recusa se `unloading` ou `inOwnEnd`; recusa (sem tocar) se `target()` existe. **Arma o ouvinte antes** de `beginDragTarget` (vê o `ended` de uma recusa interna, F4). Possui só se, depois da chamada: nenhuma `ended` ocorreu (`generation == G0`), `target()` não é nulo e é o alvo pedido. Senão desarma e devolve `REFUSED`.
- **C2 — Invalidação.** O ouvinte, em **qualquer** `ended`: `++generation`, zera posse e solta **a si mesmo** (seguro por F8). Atribuição conservadora: como `ended` não identifica a sessão, toda `ended` após a aquisição invalida.
- **C3 — Posse vigente** = ouvinte armado ∧ `ownedAt == generation` ∧ `ownedTarget` vivo ∧ `target() == ownedTarget`.
- **C4 — Fim (K3/K9).** Só se C3; marca `inOwnEnd` durante `endDragTarget()`. Sem posse: **não faz nada** (o host encerra no próximo botão/tecla, F7).
- **C5 — Rearm** = C4 seguido de C1, no mesmo quadro; se C4 não encerrou (sem posse), não há rearm.
- **C6 — Falha ao armar** (`listen` lança): o modelo explora iniciar **sem** posse (preserva o início de hoje); o plugin nunca encerra esse drag. **Política ainda não aprovada:** isso não prova Q4 no unload, pois uma interação do plugin pode permanecer ativa sem posse verificável. Alternativa: recusar o início e fazer rollback das flags dos chamadores. O dono deve decidir antes de produção. Exceções dentro de `beginDragTarget`/callbacks do host não têm rollback provado por este modelo.
- **C7 — Unload passo 2:** `unloading = true`; se C3, C4; depois **sempre** solta o ouvinte (mesmo armado por um alvo morto em silêncio), antes de remover ouvintes/decorações/timers e de qualquer `dlclose`.

**Premissas (limites da API):** A1 thread única (H-THREAD); A2 = F1 (verificado); A3 nenhum componente chama `beginDragTarget` com drag ativo (host verificado em F2; **plugins de terceiros: premissa**); A4 `ended` síncrono, uma por fim com alvo vivo (F3, verificado); A5 o unload não ocorre de dentro de uma emissão de `ended` (Q3/H-Q3, F8).

**Geração — conclusão verificada:** `ended` sem argumento só permite saber "alguma sessão terminou depois da aquisição". Com ouvinte **por sessão**, a geração é **redundante** (o próprio desarme já invalida; mutantes q09–q12 sobrevivem por isso), mas é ela que dá o retorno correto de `begin` na recusa sem reset de alvo (F5; mutante q13). Com ouvinte **permanente** (P7 §5), a geração é **essencial** (q14, q15). Proposta: ouvinte por sessão + geração como defesa.

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
| durante `dragEnd` do host | callback de outro inicia drag | C3 falso | drag do outro intacto | F6; S7 |
| durante C4 (`inOwnEnd`) | callback tenta `begin` do plugin | recusa | nenhuma sessão órfã | S8 |
| POSSUI | `begin` de terceiro **sem guarda** no mesmo alvo | indistinguível | **o plugin encerraria** | violação de A3 (teste documenta) |
| qualquer | unload | C7 | ouvinte solto; drag do outro intacto | S11–S13 |
| — | `listen` falha | C6 | drag iniciado, sem posse | S14 |

## 4. Provas host-free (`evidence/Q4A-*`)

Modelo (`Q4A-tests/q4model.hpp`, suíte `q4test.cpp`, runner `run.sh` no mesmo diretório): réplica do fluxo de `dragBegin`/`dragEnd` 0.57 (linhas citadas em F1–F6) com o **sinal real** do hyprutils (`CSignalT<>`, `CHyprSignalListener`, `CScopeGuard`, biblioteca instalada). GCC 16.2.1 e Clang 23.1.4, `-fsanitize=address,undefined`, 0 avisos, saída idêntica: **45 OK, 0 falhas** (`Q4A-run.txt`).

- **Contraexemplo reproduzido:** as três políticas sem posse (incondicional, igualdade de alvo, alvo+modo) encerram o drag do usuário na mesma janela (S2); a proposta e a variante permanente o mantêm.
- Proposta e variante permanente passam S1–S10 (S4 em 4 variantes); premissa A3 demonstrada como limite; semântica do hyprutils medida (autodesarme no próprio handler sem erro do ASan; ouvinte novo não chamado na emissão corrente; ouvinte solto antes da vez é pulado); unload (posse própria, drag de terceiros, alvo morto em silêncio, nenhum início após o passo 2); falha de `listen`.
- **Callback antigo e rearm:** um listener anterior inicia sessão nova dentro de `ended`; o snapshot antigo perde a referência do dono e é pulado pelo sinal real, sem apagar a posse nova. Rearm do drag próprio adquire sessão nova; se terceiro inicia durante o fim, o início seguinte recusa e não adota o drag de terceiro. Esses 4 checks foram acrescentados após montar o pacote da revisão.
- **Limite Q3 reproduzido:** se o módulo solta o ouvinte **de dentro** de uma emissão de `ended`, o estado do handler só é destruído no fim da emissão (depois do "unload") — o `std::function` do plugin roda o destrutor fora do controle do plugin. Exige A5.
- **Mutações do modelo** (`Q4A-mutations.log`): 15; **10 mortas** (guarda de drag ativo, `inOwnEnd`, `unloading`, desarme no handler, soltura no unload, política de falha, posse = "há alvo", geração na recusa, e as duas checagens de geração da variante permanente); **5 sobrevivem como redundância explicada** (geração e alvo vivo em C3, igualdade de alvo sob A3, ouvinte nulo = desarmado, verificação antes de C4 no unload). Duas lacunas reais apareceram (q05: unload com alvo morto em silêncio; q15: recusa sem reset de alvo) e ganharam teste.
- Um erro do **próprio teste** (callback que se apagava durante a execução) foi achado pelo ASan e corrigido no modelo (cópia antes de invocar), sem mudar a semântica.

**Limites:** réplica manual, não o `DragController` real; não executa `updateDragWindow`, foco, grupos, decorações reais, nem o laço do Hyprland; não prova A1, A3 (terceiros) nem A5; nenhum teste aninhado ou ao vivo. As sondas reais (`Q4A-tests/probe.cpp`) passam GCC/Clang; negativo falha somente pelo membro `sessionId` ausente (não prova sozinho a ausência de qualquer API equivalente; essa conclusão vem da inspeção do header).

## 5. Reentrada e descarregamento

- **Reentrada no fim:** entre o reset do alvo (:274) e `ended` (:258/saída) rodam callbacks (F6). Um início reentrante de terceiro não é encerrado pelo plugin (S7). Início reentrante do plugin: recusado em C4 por `inOwnEnd` (S8); durante um `dragEnd` **do host** o plugin não tem caminho de início conhecido (F9) — invariante a manter, não verificável pelo plugin.
- **Autodesarme** dentro do handler é seguro (F8, S-listener com ASan).
- **Ordem no unload (C7):** passo 2 antes de remover os demais ouvintes/decorações; o `endDragTarget()` do passo 2 é síncrono e roda os callbacks de F6 **com o módulo mapeado** (decorações, `window.active` do plugin) — o passo 2 não pode agendar trabalho diferido (tooltip, rearm, preview) com `unloading`. Ouvinte solto antes do fim de `PLUGIN_EXIT`.
- **Relação com Q1–Q5:** Q4 = "nenhuma interação dependente do módulo ativa, ou encerrada pelo passo 2" (`QUIESCENCE.md` §2): este contrato define **qual** drag o passo 2 pode encerrar (só o próprio) e reduz L17 (`LEDGER.md`) a "drag próprio encerrado; de terceiros intacto". Q2 (contador de entradas) deve contar o handler de `ended`; Q3/H-Q3 cobre A5 (unload fora de emissão de `ended`); Q5 inalterado. O unload administrativo quiescente vem do IPC (fora de emissões, `QUIESCENCE.md` §1), compatível com A5; o caminho `reload()` também ocorre fora das emissões de config, mas **não** foi verificado contra emissões de `ended` (nenhuma `ended` é emitida por `reload()` nas fontes lidas — hipótese a confirmar).

## 6. Critérios para autorizar a implementação de Q4-A

1. Revisão independente deste contrato sem blocker/major.
2. Dono aceita as premissas A3 (terceiros) e A5 (unload fora de emissão de `ended`) como limites documentados, e a perda conservadora de posse (C2/F5).
3. **Não integrar listeners de produção sem autorizar e provar também sua soltura no lifecycle.** Estado C1–C3, K3/K9/rearm e C7 precisam de autorização explícita e coordenada; não basta implementar a posse e adiar a remoção do listener. Enquanto `PLUGIN_EXIT` permanecer fora do escopo, só protótipo host-free. Decidir C6 (falha de `listen`), política de exceções e rollback das flags antes da integração.
4. Antes do código: as provas desta etapa viram testes do repositório (vermelho primeiro); depois: equivalência com host falso contra o corpo 0.56.2 nos caminhos de fim (com o drag próprio), negativos de terceiros, mutações, sondas de tipo (`m_events.ended`, `CHyprSignalListener`), comparação de erros.
5. Teste aninhado/ao vivo do dono antes de declarar Q4 resolvido: S2 real (atalho do usuário após tecla), unmap durante drag, rearm com Shift, unsnap, unload com drag ativo.

## 7. Revisão independente

**Bloqueada; nenhum parecer independente obtido.** Tentativas em processos separados, somente pacote de fontes/contrato/testes, sem ferramentas de escrita:

| Provedor/modelo | Resultado real |
|---|---|
| Anthropic `claude-sonnet-4-6` | "Execution outcome is unknown"; sessão registrada, sem resposta de revisor recuperável; não reenviado à mesma sessão |
| Gemini `gemini-2.5-pro` | HTTP 404: modelo indisponível para novos usuários; API recomendou `gemini-3.1-pro-preview` |
| Gemini `gemini-3.1-pro-preview` | HTTP 429 `RESOURCE_EXHAUSTED`, quota free-tier 0 |

Provedor ChatGPT não utilizado nesta etapa, respeitando a indisponibilidade informada pelo dono. CLI de Claude/OpenCode/Codex não disponível no PATH; nada instalado, nenhuma configuração global alterada. Falhas preservadas em `evidence/Q4A-review-attempts.txt`. O pacote enviado está preservado como snapshot em `Q4A-review-prompt-1.txt`; os testes adicionados e as ressalvas posteriores **não foram revisados**. Resultado: material preparatório reproduzível, **não aprovado para implementação**; Q4 permanece aberto.
