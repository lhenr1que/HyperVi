# Plano: `kwinish`, gerenciador de janelas estilo KWin para qualquer Hyprland

Nome provisório: **kwinish** (troque à vontade). Status: **plano, aguardando ok do dono. Nenhum código escrito.**
Base lida: `BRIEF-HYPR-WM-KWIN.md`, `kwin-vs-hyprland-marcado.md`, `ORQUESTRACAO.md`, `REVISAO-GEMINI-KWIN.md`.
Só existe Hyprland 0.56.2 nesta máquina. Tudo que digo sobre outras versões é hipótese a verificar, e está marcado assim.

## 0. Ideia central

Os itens que você marcou se dividem em três grupos, e só o primeiro precisa de programa novo:

1. **Comportamento que o Hyprland não tem** (encaixe, mostrar área de trabalho, manter acima, maximizar num eixo, encaixe ao arrastar, posicionamento inteligente, auto-raise, Alt+Tab, cantos ativos). É o miolo do programa.
2. **Coisa que o Hyprland já faz** (regras, snap de borda, dim, blur, grupos, VRR, ANR, `no_screen_share`...). Não se reescreve; entra como **receitas** (trechos de config prontos e documentados) e como página de ajuda, nada de código.
3. **Coisa que depende de UI ou de plugin C++** (editor de zonas, temas de decoração, efeito de minimizar). Fica para depois.

## 1. Escopo: o que entra na v1 e o que fica para depois

Números = linhas de `kwin-vs-hyprland-marcado.md`.

### v1 (daemon + comandos, sem UI própria, exceto o mínimo de Alt+Tab)

| # | Recurso | Como na v1 |
|---|---|---|
| 5, 15 | Quick tile (metades, cantos, topo/base), empacotar, crescer/encolher pelo teclado | comandos `kwinish tile <dir>`, `pack`, `grow` |
| 4 | Maximizar só na horizontal/vertical | `kwinish maximize h\|v` (janela flutuante; pedir de novo restaura) |
| 25 | Mostrar a área de trabalho | `kwinish show-desktop`; origem guardada em tag da janela (sobrevive a reload de config, como no Ryoku) |
| 2, 3 | Manter acima (e abaixo) | `kwinish keep above\|below`; daemon reordena no evento de foco |
| 7 | Encaixe ao arrastar para a borda | daemon (ver risco R3) |
| 9 | Posicionamento: cascata, inteligente, centrada, sob o mouse (aleatória e canto: fácil, entram junto) | daemon no evento `openwindow` |
| 10 | Auto-raise com atraso, anti-roubo de foco | daemon; foco por clique/seguir mouse já é nativo (`follow_mouse`) |
| 26 | Alt+Tab com teclado (ordem de foco, minimizadas, soltar Alt escolhe) | daemon guarda a ordem; **UI e soltura de tecla são o risco R1; começa por um spike** |
| 24 | Cantos/bordas ativas | daemon + hotspots (ver R5) |
| 12 | Menu da janela | `kwinish menu` chama um seletor externo configurável (`fuzzel`/`wofi`/`rofi`/`tofi`/`walker`, o primeiro que existir); sem Quickshell |
| 1, 8, 13, 14, 17, 18, 21, 22, 29-31, 36, 44, 48, 49, 55, 58 | Nativos do Hyprland | **receitas** em `recipes/` (config pronta, hyprlang e Lua) |
| 19 (parte) | Áreas de trabalho: comandos de grade 2D (`desk up/down/left/right`) sobre workspaces numerados | v1 simples; grade completa depois |
| 16 (parte) | Botões da barra de título configuráveis | só como receita do plugin hyprbars (que é plugin de terceiro, opcional) |
| 32 (parte) | Uma animação simples de abrir/fechar | receita de `animation =`, sem código |
| 51 | Atalhos | `kwinish binds print` gera as linhas prontas para colar (ver §3) |

### Depois (marcado "para depois" ou depende de UI/C++)

6 (zonas e editor de ladrilhos), 16 (temas de decoração), 32 (Glide/Squash/Sheet), 33 (efeito ao minimizar; minimizar em si entra na v1 como mover para workspace especial), 41 (filtros de daltonismo por shader; inverter é uma linha de shader, entra como receita), 52 (gestos de borda), 53 (teclado virtual/tablet), 54 (gamepad), 19 (grade 2D completa), 59 e 56 ("repensar depois" / "cogito depois").

### Fora (você marcou Não, ou não faz sentido)

20, 35, 42, 45, 50, 57, 62, e 46/60 (você não sabe o que são; resumo: 46 é leitor de tela e sino do sistema, 60 são scripts de efeito do KDE escritos em JS/QML. Nenhum dos dois faz falta para o seu uso; sigo sem eles).

## 2. Arquitetura

```
 teclado/mouse ──► Hyprland (binds do usuário) ──exec──► kwinish <ação>   (CLI, cliente fino)
                                                              │ unix socket
 Hyprland .socket2 (eventos) ─────────────────────────► kwinishd  (daemon)
 Hyprland .socket  (comandos/dispatch/keyword) ◄────────────────┘
```

**Uma linguagem, um binário: Go.** Motivos: o Ryoku já tem a camada `ryoku/wm` em Go com cliente do socket do Hyprland e watcher de eventos que posso consultar como referência; binário estático, sem runtime; instala em qualquer distro. O programa novo **não importa** código do Ryoku (copio a ideia, não a dependência).

Peças e por que cada uma existe:

| Peça | Roda onde | Por que não pode ser outra coisa |
|---|---|---|
| **Binds** | config do usuário (`exec, kwinish tile left`) | O Hyprland é dono das teclas. Funciona igual em hyprlang e em Lua, porque só chama um executável |
| **CLI `kwinish`** | processo curto por atalho | Só serializa o pedido e manda ao daemon; sem estado |
| **Daemon `kwinishd`** | processo longo (serviço de usuário ou `exec-once`) | Precisa de estado e de eventos: ordem de foco, origem das janelas no "mostrar desktop", camadas "manter acima", posicionamento ao abrir, auto-raise, fim de arrasto. Config estática não guarda isso |
| **Comandos ao Hyprland** | socket `.socket` (dispatch, `setprop`, `keyword`) | Evita depender de `hyprctl` instalado no PATH e de Lua; é a mesma interface que o `hyprctl` usa |
| **Overlay do Alt+Tab** | processo próprio (layer-shell) | Só há UI aqui por causa das miniaturas e da soltura de tecla (R1). Fase 1: lista de texto/ícones; miniaturas por `grim -g`/`hyprland-toplevel-export` ficam para uma fase 2 |
| **Config do kwinish** | `~/.config/kwinish/config.toml` | Um só arquivo, lido pelo daemon (recarga por `kwinish reload`). **Não toca em `hyprland.conf`/`.lua` do usuário** |

Dentro do Hyprland (config) fica **só** o que é nativo: regras, animações, `general.snap`, `follow_mouse`. Isso vai nas receitas, que o usuário cola, nunca o programa escreve sozinho.

Sem UI para tudo o resto na v1. Quickshell/Ryoku ganham integração opcional depois (o Ryoku passaria a chamar `kwinish` em vez da sua cópia em Lua), mas o programa nunca depende deles.

## 3. Instalação em qualquer Hyprland

Princípio: **nenhuma dependência de Ryoku, Quickshell, plugin ou Lua.** Três passos que o usuário vê e controla:

1. **Binário**: pacote AUR (`kwinish-git`/`kwinish-bin`), release estática do GitHub, ou `go install`. Dependências de runtime: nenhuma obrigatória; opcionais: um seletor (menu da janela) e `grim` (miniaturas).
2. **Serviço**: `kwinish install --service` grava uma unidade systemd de usuário; alternativa sem systemd: uma linha `exec-once = kwinishd`.
3. **Binds**: `kwinish binds print --format lua|conf` imprime as linhas, e o usuário cola onde quiser (ou `kwinish binds apply` escreve um arquivo separado `kwinish-binds.{conf,lua}` e diz como dar `source`/`require`). Nunca edita o arquivo principal.

**Por que não plugin `hyprpm`:** plugin C++ quebra a cada versão do Hyprland (ABI), exige headers, compilação e rodar `hyprpm update`. Os recursos da v1 não precisam disso. Plugin só entra se um item "G" for aprovado depois (zonas, efeito de minimizar) e será opcional e separado.

**Versão mínima (hipótese, a validar no spike):** a v1 usa só IPC (`.socket`, `.socket2`, `dispatch`, `setprop`, tags, workspaces especiais), que existe há muito tempo. Proposta: **declarar suporte oficial a 0.55+ (o que realmente testo: 0.56.2 aqui) e marcar versões mais antigas como "provavelmente funciona, não testado"**, subindo o piso só se o spike achar um dispatcher que falta. Hyprland em hyprlang (<0.55) e em Lua (0.55+) são cobertos pelo mesmo binário, porque os binds só chamam o CLI. Não tenho como testar hyprlang aqui sem instalar outra versão; fica como tarefa de CI (container com Hyprland antigo), não como promessa.

O daemon detecta a versão no start (`hyprctl version` via socket) e **desliga por conta própria** o recurso que depender de algo ausente, avisando no log, em vez de falhar.

## 4. Riscos

| # | Risco | Mitigação / plano |
|---|---|---|
| **R1** | **Alt+Tab e soltura de tecla.** Lições do `ORQUESTRACAO.md` (T7): no 0.56.2 nenhuma soltura de tecla dispara dentro de submap (tentativa 1); foco de teclado exclusivo no overlay deu 3 passos por Alt+Tab, provável reenvio de teclas ao ganhar foco (tentativa 2). | **Spike antes de qualquer outra coisa do Alt+Tab**, três candidatos medidos em ordem: (a) overlay layer-shell com foco exclusivo que **ignora eventos até o primeiro release do modificador**, mais deduplicação do key-repeat; (b) bind global ativado só enquanto o overlay está aberto (`ALT+Escape`/setas), e fechamento por temporizador de segurança (o de 15 s que salvou o teclado antes); (c) bind `release`/`bindr` do modificador fora de submap. Critério: um Alt+Tab = exatamente um passo, soltar Alt confirma, Esc cancela, e **nunca prende o teclado** (temporizador obrigatório). Se nenhum passar, o Alt+Tab sai da v1 e vira "ciclar sem soltura" (cada atalho troca de janela direto), que é útil e seguro. |
| **R2** | **Diferenças entre versões do Hyprland** (Lua vs hyprlang, nomes de dispatchers, formato de eventos, `alter_zorder` etc.). Hyprland muda rápido. | Só IPC; detecção de versão e recursos desligados individualmente; suíte de testes de contrato do IPC contra um Hyprland headless (`WLR_BACKENDS=headless`) por versão no CI; documentar a matriz testada. |
| **R3** | **Fim do arrasto.** O Hyprland não emite evento de "soltou a janela" (o Ryoku usa um timer de 60 ms que dispara com o botão ainda pressionado, achado 3 da revisão do Gemini). | Spike: medir o que há em `.socket2` (`movewindow`, `activewindow`) e se `hyprctl cursorpos` + estado do botão permitem detectar o fim sem privilégios. Alternativa: `bindm` com `release` quando o usuário arrasta com Super. Se não houver forma limpa, o encaixe ao arrastar entra marcado **experimental**. Não ler `/dev/input` (exigiria grupo `input`, e é invasivo). |
| **R4** | **Corridas e loops**: o daemon reage a eventos e também emite comandos (por exemplo, reordenar z ao focar gera novo evento de foco). | Máquina de estados simples com "ignorar o próximo evento que eu mesmo causei" e teste com eventos simulados. |
| **R5** | **Cantos ativos sem UI**: precisam detectar o cursor no canto. Polling de `cursorpos` gasta CPU; layer-surface invisível de 1 px exige Wayland client. | Começar com polling adaptativo (lento fora de borda, só enquanto o cursor se move) e medir CPU; se passar de ~1% constante, trocar por uma layer-surface fina (a mesma biblioteca do overlay do R1). |
| **R6** | **Não pisar na config do dono**: tags, regras e estado do daemon podem colidir com regras do usuário. | Prefixo único (`kwinish:`) em tags; nada escreve em arquivos do usuário sem comando explícito; `kwinish doctor` lista colisões. |
| **R7** | **Teste na tela única do dono.** Mexer em janelas reais é proibido. | Todo teste em janela descartável (`foot`/`kitty` com título fixo) mais a suíte headless; testes na tela real só em fila, com `grim` e `pointer-path.py`, e guardam/restauram o estado. |
| **R8** | **Múltiplos monitores** (nunca testado nas rodadas anteriores). | Cálculo de geometria sempre pelo monitor da janela e por `reserved` (barras); testes headless com 2 monitores virtuais. |
| **R9** | **Duplicar o que o Ryoku já tem** e criar dois caminhos para a mesma coisa. | Decisão pendente (pergunta 3): o Ryoku migra para o `kwinish` depois, ou o programa nasce paralelo e independente. |
| **R10** | **Cota do agy** esgotada até ~28/10 (e a lição de não passar de 2 tarefas grandes). | Nada urgente; a v1 pode ser feita por mim com subagentes, ou esperar a cota. |

## 5. Fases e critério de pronto

0. **Spikes (1 a 2 dias):** R1 (Alt+Tab), R3 (fim de arrasto), conferir lista de dispatchers/IPC necessários no 0.56.2. Saída: relatório curto e decisão do que entra de fato.
1. **Núcleo:** daemon, CLI, config TOML, `doctor`, `binds print`. Comandos: tile, pack, grow, maximize h/v, show-desktop, keep above/below, posicionamento, auto-raise. Teste automatizado com Hyprland headless.
2. **Interação:** encaixe ao arrastar, cantos ativos, menu da janela, comandos de grade de áreas de trabalho.
3. **Alt+Tab** conforme o resultado do spike.
4. **Receitas e empacotamento:** `recipes/`, PKGBUILD, unidade systemd, README.

**Pronto da v1 no seu desktop:** instalar numa conta limpa/sem Ryoku só com o binário + as linhas coladas; cada item da tabela da §1 passa num roteiro de teste com janela descartável; `kwinish doctor` sem erros; nenhum arquivo seu alterado fora do que o instalador listar.

## 6. Perguntas para você (respondam antes de eu começar)

1. **Nome e repositório:** `kwinish` serve? Repositório novo e separado (não dentro de `ryoku-fork`), certo? Lembrando: nada de push, fork ou PR sem seu ok explícito; começo só com repo local.
2. **Piso de versão:** aceitar "0.55+ oficial, resto não testado" na v1?
3. **Relação com o Ryoku:** o programa nasce independente e o Ryoku adota depois (recomendo), ou o Ryoku continua com a cópia em Lua e os dois divergem?
4. **Alt+Tab:** concorda em tratar como spike primeiro, com a saída "ciclar sem soltura" caso nada passe?
5. **Linguagem:** Go ok? (A alternativa seria Lua puro dentro do Hyprland, que não serve para "qualquer Hyprland" e não guarda estado entre reloads.)
6. **Menu da janela** por seletor externo (fuzzel/rofi etc.) na v1, em vez de UI própria: aceitável?

## O que não conferi

- Comportamento do IPC em versões diferentes de 0.56.2 (só essa está instalada).
- Se o Hyprland 0.56.2 expõe algum evento de fim de arrasto; é o primeiro item do spike.
- Os itens "Nativo" da tabela vêm do catálogo; não os retestei nesta sessão.

## Revisão 1 (respostas do dono, 23/10/2026)

- **Q1, Q2, Q4:** ok. **Q3:** independente; o Ryoku não é obrigado a adotar; pacote AUR opcional para quem quiser.
- **Q5 (linguagem), dado novo:** a wiki oficial diz "Desde o Hyprland 0.55, o hyprlang está obsoleto em favor do Lua" (wiki.hypr.land, páginas Start e Expanding functionality), com `hl.on` para eventos e `hl.timer`. Logo: (1) o piso "0.55+" deixa de ser limitação, é o caminho oficial e o hyprlang vai morrer; (2) a lógica de janelas pode viver **dentro do Hyprland, em Lua**, sem daemon, como um módulo que o usuário carrega com `require`. Isso derruba a premissa de Go para o núcleo.
- **Q6:** UI própria desde o começo. Lua não desenha nada, então a UI é um programa à parte (layer-shell) em qualquer cenário.

### Arquitetura revisada (proposta)

| Peça | Linguagem | Motivo |
|---|---|---|
| **Núcleo** (tile, pack, grow, maximize h/v, show-desktop, keep above, posicionamento, auto-raise, hot corners por evento) | **Lua**, módulo `kwinish.lua` + `require("kwinish").setup{...}` | Roda no processo do Hyprland, vê os eventos direto, zero IPC e zero processo extra; é a direção oficial. Estado entre reloads via tags de janela (já provado no Ryoku) |
| **UI** (Alt+Tab com miniaturas, menu da janela, cantos ativos se precisar de superfície) | **Rust** (smithay-client-toolkit ou gtk4-layer-shell), um binário `kwinish-ui` | Layer-shell com foco exclusivo e captura é o que o Lua não faz. Go não tem toolkit layer-shell maduro; Rust é o que o ecossistema Hyprland mais usa para isso. Comunica com o núcleo por `hyprctl dispatch`/socket |
| **Instalação** | PKGBUILD: instala `kwinish.lua` em `/usr/share/kwinish/` e o `kwinish-ui`; `kwinish binds print` gera as linhas Lua | Sem hyprlang: a v1 mira só Lua 0.55+ |

Custo honesto: são duas linguagens (Lua e Rust) em vez de uma. Em troca, o núcleo fica pequeno e sem daemon. Risco novo a validar no spike: o **Lua do 0.56.2 não vê o fim do arrasto nem soltura de tecla em submap** (já visto no Ryoku); a UI em Rust cobre o Alt+Tab, mas o fim do arrasto continua em aberto (R3).
O ponto de decisão que sobra para o dono: confirmar "Lua no núcleo + Rust na UI" (recomendado) ou "tudo em Rust/Go com daemon".
