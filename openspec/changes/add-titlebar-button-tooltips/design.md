# Design

## Context

A barra é uma decoração `DECORATION_LAYER_UNDER`: tudo o que ela desenha fica **abaixo** da superfície da própria janela, então uma dica desenhada no `draw()` da barra, abaixo do botão, seria coberta pelo conteúdo da janela. O hover já é calculado em cada movimento do ponteiro (`damageOnButtonHover`, `layoutButtons`). O plugin evita callbacks adiados que possam sobreviver ao descarregamento (comentário do Shift em `main.cpp`). O Hyprland 0.56.2 emite `render.stage` com `RENDER_POST_WINDOWS` enquanto monta o passe, depois de todas as janelas (fixadas incluídas) e antes das camadas `top`/`overlay` (`src/render/Renderer.cpp` da tag v0.56.2); o hyprwinwrap oficial já acrescentava elementos ao passe nesse evento.

## Goals / Non-Goals

**Goals:**
- Uma dica por vez (há um ponteiro só), por cima de todas as janelas, sem opção nova.
- Lógica pura (texto pelo locale e estado maximizado, posição dentro do monitor) testável sem Hyprland.

**Non-Goals:**
- Dica nos botões de fixar, `exec:` e no ícone; dica nas barras de reserva (`kwinish-ui bars`); animação de entrada; sombra.
- Ficar acima das camadas do shell (`top`/`overlay`): elas ficam por cima, como já ficam por cima das janelas.

## Decisions

- **Desenhar em `RENDER_POST_WINDOWS`, com elementos do próprio Hyprland** (`CRectPassElement` para a caixa e a borda, `CTexPassElement` para o texto). Alternativas: uma segunda decoração `DECORATION_LAYER_OVERLAY` por janela (fica acima só da própria janela, coberta pelas da frente, e dobra o ciclo de vida das decorações); uma superfície layer-shell pela `kwinish-ui` (processo novo a cada hover, atraso visível). Elementos do Hyprland também dispensam `removeAllOfType` no `PLUGIN_EXIT`, porque não há vtable do plugin neles.
- **Estado global único** em `g_pGlobalState->tip`: a barra dona (`WP`), o índice do botão, o texto, o retângulo do botão e do monitor, se já passou o atraso, a textura. A barra que tem o ponteiro sobre um botão com dica toma posse; a que perde o ponteiro solta. Para duas barras sobrepostas não disputarem a posse, só toma posse a barra cuja janela está de fato sob o ponteiro (o mesmo teste de `inputIsValid`, exigindo a janela sob o ponteiro e não só a focada).
- **Atraso por `CEventLoopTimer`** (700 ms, o atraso padrão das dicas do Qt/KDE), um só, criado na primeira vez e **removido do `g_pEventLoopManager` no `PLUGIN_EXIT`**: o gerenciador guarda referência forte e chamaria/destruiria um `std::function` do plugin já descarregado. O callback só lê `g_pGlobalState`.
- **Dano do monitor inteiro ao mostrar e ao esconder**, como a prévia do encaixe já faz: a caixa só é medida quando a textura é criada, dentro do render (contexto GL válido); mostrar/esconder acontece uma vez por hover.
- **Esconder em qualquer aperto de botão do mouse** (o ouvinte global de `mouse.button` em `main.cpp`), sem soltar a posse: o ponteiro ainda está no botão e a dica não volta até sair e entrar de novo (KDE). Arrasto começa com um aperto, então também esconde.
- **Texto**: tabela por ação em código, português se o primeiro valor não vazio de `LC_ALL`, `LC_MESSAGES`, `LANG` começa por `pt`. A UI Rust usa a mesma ordem, mas um `LC_ALL` vazio ali conta como inglês; aqui um valor vazio é pulado (POSIX). Maximizar numa janela já maximizada diz Restaurar/Restore.
- **Aparência**: fonte e tamanho do título da barra, fundo `bar_color` com alfa mínimo de 0,95 (legível sobre qualquer conteúdo), texto `col.text`, borda de 1 px do texto com alfa 0,25, cantos de 4 px, 8×4 px de margem interna, 4 px abaixo da barra; sem espaço abaixo, vai para cima da barra; sempre presa dentro do monitor.

## Risks / Trade-offs

- [`RENDER_POST_WINDOWS` não é emitido num quadro de varredura direta (tela cheia com scanout)] → nessa situação não há barra visível para passar o mouse.
- [Janela focada coberta: o teste de posse exige a janela sob o ponteiro, mas o clique continua com a regra antiga (pendência 2 da revisão)] → a dica nunca aparece para a barra de trás; o clique não muda nesta mudança.
- [Só o dono pode carregar o plugin] → a lógica pura tem teste próprio; o desenho, o atraso e o descarregar com a dica na tela vão para "Testes pendentes para o dono".
