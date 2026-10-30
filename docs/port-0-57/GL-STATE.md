# Estratégia de estado OpenGL (tarefa 1.4, parte B2)

Preparatório. Não executa GL. Distingue **parâmetro** (P), **conteúdo** (C) e **cache do host** (K). Legenda de evidência em [README](README.md). Complementa, sem repetir, `investigacao-b1-b3/B2-RENDER-OPENGL.md`.

## 1. Fatos novos sobre o estado herdado [N]

1. **O stencil é scratch por convenção do upstream.** Em todo `src` do 0.57, `glClear(GL_STENCIL_BUFFER_BIT)` aparece **uma vez** (`U:render/OpenGL.cpp:1894`, `renderTextureWithBlurInternal`; busca `glClear(GL_STENCIL` em todo `src`: 1 ocorrência), seguido de uso e `setCapStatus(GL_STENCIL_TEST,false)` (:1975). O único outro toque é `Kawase.cpp:69` (desliga o teste). Nenhum consumidor lê conteúdo de stencil deixado por elemento anterior.
2. **O stencil é compartilhado por monitor.** `MonitorResources` cria um `m_stencilTex` e o anexa aos buffers do monitor (`U:output/MonitorResources.cpp:22,28,47`) e ao FB de blur (:194; desanexado em :191). Um `glClear` num alvo é visível por todos os FBs do monitor que o compartilham.
3. **O upstream não depende de máscara herdada.** Antes do clear ele fixa `glStencilMask(0xFF)` (:1892). Ao sair deixa `glStencilMask(0x00)`, `glStencilFunc(EQUAL,1,0xFF)` e `glStencilOp(KEEP×3)` (:1923–1925), apenas com o *teste* desligado (:1975). Tudo isso só no ramo `!SHADERBLEND && data.blurredBG && NEEDS_STENCIL` (:1885–1888): depende de `render:use_shader_blur_blend` e dos flags do elemento. Logo, **nesse ramo os parâmetros herdados não são os default.**
4. **Consequência para o baseline (H-GL1).** O recorte da barra (`barDeco.cpp:1037–1052`) não fixa `glStencilMask` antes de `glClear` nem de `GL_REPLACE`. `glClear` respeita a máscara de escrita do stencil: sob `0x00` o clear e o REPLACE seriam no-ops e o recorte NOTEQUAL deixaria de excluir a janela. Só ensaio decide.
5. **O scissor do plugin é inócuo para os desenhos (H-GL2).** `renderRect` e `renderTexture` iteram os retângulos de damage chamando `scissor(ctx,&RECT,…)` e terminam em `disableScissor()` (`U:OpenGL.cpp:1252,1258,1264,1282,1837,1873,2005`; `GLElementRenderer.cpp:31,35`). O `scissor(titleBarBox)` de `barDeco.cpp:1024` só limita o **primeiro** `glClear` (`:1038`). O segundo (`:1068`, "cleanup") roda **depois** de `renderRect` e, portanto, com scissor desabilitado: por leitura estática ele limpa o **attachment inteiro**, não a caixa da barra. Isto corrige a leitura de que "limpar só um retângulo" seria o dano.
6. **`renderRect` do 0.57 tem campos novos de blur** (`H:render/OpenGL.hpp:137–147`): `blurPatternBox`, `blurOwner`, `workspacePresentation`, repassados ao planejador de blur (`U:OpenGL.cpp:1170,1185`). O baseline passa só `blur`/`blurA`. Omitir `workspacePresentation` e `blurOwner` pode planejar o blur com apresentação default (H-GL5).
7. **`createTexture(cairo_surface_t*)` e `renderText` rodam dentro do draw** (`barDeco.cpp:802,856,919–923,1297`), fazem `makeEGLCurrent()` e deixam textura vinculada (`U:GLRenderer.cpp:280–305`). Efeito no draw seguinte: não demonstrado (H-GL4).

## 2. Matriz de estado

| Estado | Classe | Quem altera no baseline | Dono / convenção no 0.57 | Estratégia |
|---|---|---|---|---|
| Scissor (enable + caixa) | P + K | `scissor(titleBarBox)` `:1024`, `scissor(nullptr)` `:1033,1103` | cache `m_capStatus[SCISSOR]` e `static CBox m_lastScissorBox` **local à função** (`U:OpenGL.cpp:1111`); cada primitiva termina com scissor desligado | Usar só `scissor(ctx,…)`/`disableScissor()`. Convenção a preservar: **desligado entre elementos**. Não restaurar "recorte herdado" (design 5) |
| Stencil `TEST` | P + K | `setCapStatus(GL_STENCIL_TEST,…)` `:1040,1070` (passa pelo cache) | idem | Só via `setCapStatus`; nunca `glEnable` cru |
| Stencil func/op/ref/mask, clear value | P (sem cache) | `glStencilFunc/Op/Mask`, `glClearStencil` `:1037,1042–1043,1051–1052,1068,1071–1072` | scratch; ver fatos 1 e 3 | Ver ladder E0/E1 |
| **Stencil conteúdo** | **C** | `glClear` `:1038,1069`; REPLACE na máscara | scratch compartilhado por monitor | Não é salvável por guarda alguma (B2 §2). **Só se evita** (E0) ou **se declara scratch** (E1) |
| Color write mask | P (sem cache) | `glColorMask` `:1045,1049` | padrão RGBA=true | Evitar (E0); em E1 salvar/restaurar os 4 canais por `glGetBooleanv` em todo retorno |
| Blend | P + K | internos de `renderRect` | `blend(bool)` fixa fatores premultiplicados (`U:OpenGL.cpp:1088–1096`) | Não tocar; helpers do host decidem |
| FBO/viewport | P + K | nenhum direto | `CFramebufferBindingGuard`, `bindTempFB` | Não tocar (sem FBO privado em E0/E1) |
| Textura vinculada / unidade ativa | P + K | `createTexture`, `renderText` | host rebinda antes de usar [a confirmar: H-GL4] | Criar texturas fora do caminho crítico quando possível; D10 compara draw seguinte |
| Dados lógicos (`m_data`, damage, clip, renderModif) | dados | helpers | `ctx.saveDrawState()` restaura **só isto** (`H:Context.hpp`) | Usar se o plugin alterar `ctx.m_data`; E0 não altera |

## 3. Escada de estratégias (regra de decisão)

| Nível | Descrição | Condição para escolher | Teste que decide |
|---|---|---|---|
| **E0 (preferido)** | **Zero GL cru** no plugin: sem `glStencil*`, `glClear`, `glColorMask`, `glScissor`, `glEnable/Disable`. Corte das cunhas (barra estendida menos a forma arredondada da janela) por primitivas nativas: textura de cobertura + `blurAlphaMatte`/`renderTextureMatte`, conforme B2 §4 | Equivalência visual de cantos, blur e alpha demonstrada em GL real | S9 (negativo: `grep` por GL cru) + D9/D10/D11 |
| **E1 (fallback)** | Manter a técnica de stencil como **uso scratch com colchete completo**: salvar `STENCIL_TEST`, `STENCIL_WRITEMASK`, `STENCIL_FUNC/REF/VALUE_MASK`, `STENCIL_FAIL/PASS_DEPTH_*`, `STENCIL_CLEAR_VALUE`, `COLOR_WRITEMASK` (leitura por `glGet*`/`glIsEnabled`); fixar `glStencilMask(0xFF)` antes do clear; escopo de dano restrito à caixa da barra com scissor **realmente ativo na hora do clear** (reativar após cada `renderRect`); restaurar tudo em **todo** retorno (RAII). Conteúdo: declarado scratch | E0 não equivalente **e** D9 mostra que nenhum consumidor upstream depende de conteúdo herdado | D9 (padrão conhecido, inclusive fora da caixa), D10 |
| **E2 (parar)** | Stencil privado: FB próprio com `createStencilTexture` + `createFB` e composição de volta. Mexe em projeção/fbSize/damage/renderModif e perde o backdrop do blur | Só se E0 e E1 falharem. **Não escolher sem decisão do dono** (design 5: "parar para decisão arquitetural") | D9–D11 |

Nenhum nível desliga blur ou máscara para "fechar" build (design 5, última frase).

Hipóteses de apoio: **H-GL3** (conteúdo de stencil é scratch ⇒ E1 admissível). Se D9 achar um consumidor que lê conteúdo herdado, E1 cai e resta E0 ou E2.

## 4. Caminhos de saída de `renderPass` a cobrir (insumo de 5.1/5.2)

| Saída | Baseline | Estado a repor |
|---|---|---|
| `HEIGHT < 1` | `:998–1001`, antes de qualquer GL | nenhum |
| `titleBarBox` degenerado | `:1021–1022`, **antes** de `scissor` | nenhum |
| `windowBox` degenerado | `:1032–1035` | scissor desligado (já feito em `:1033`); stencil ainda intocado |
| Fluxo normal | `:1103` | scissor, stencil (TEST, máscara, func, clear value), color mask |
| Exceção em `renderText`/`updateIcon` | `updateIcon` tem `try/catch` (`:798–806`); `renderBarTitle` não | colchete RAII em E1; em E0 nada a repor |

## 5. O que precisa de GL real (não fecha com mock)

**Retificações da revisão P2:**
- "Cleanup limpa o attachment inteiro" exige **scissor desligado e máscara de escrita permissiva**. Sob máscara `0x00`, clear/REPLACE não escrevem; a sequência herdada e o efeito visual permanecem condicionados. A busca por `glClear` sozinha não prova ausência de consumidores de stencil.
- E1 **não está aprovado** como contenção completa: limitar os clears não limita o `REPLACE` de `windowBox` se `renderRect` substitui o scissor pelo damage. Todo write precisaria ser delimitado; RAII não recupera conteúdo.
- D10 deve separar estado restaurado exatamente, estado normalizado à convenção do host (scissor desligado), coerência de caches e conteúdo scratch. A sonda não pode exigir simultaneamente scissor herdado ligado e saída normalizada desligada.
- E0 sem GL cru não prova que helpers nativos preservem conteúdo. D9–D11 continuam necessários; não houve GL real nesta tarefa.

Relatório: `evidence/P2-review-contract.txt`.

D9 (stencil com padrão conhecido, inclusive região fora da caixa e máscara herdada `0x00`), D10 (scissor/viewport/blend/color mask/bindings + caches), D11 (blur desligado/ligado, `workspacePresentation`). Ver `RISKS-TESTS.md` §3 para a estratégia de sonda aplicável.
