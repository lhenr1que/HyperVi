# Design

## Context

Base conferida em 01/11/2026: branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `ae1f250` (alpha-1 implementada em `38cc338` e arquivada).

Rotas atuais em `plugin/barDeco.cpp` que mudam geometria ou foco antes do início do drag:
- `onMouseMove`, ramo `m_unsnap.active`: `applyUnsnap` (resize + move) em cada movimento e `handleMovement()` ao fim do ease;
- `onMouseMove`, início pendente (`m_bDragPending`): saída de maximizada (`fullscreenWindow` + `move`) e `startUnsnap`;
- `handleMovement`: `fullWindowFocus` antes de `KwDrag::begin`.

Nenhuma confere `dragController()->target()`. `KwDrag::begin` (`plugin/hostDrag.hpp:36–59`) recusa com drag ativo, mas só depois de o foco ter mudado, e escolhe a janela pelo hit-test do ponteiro.

Fatos do host usados aqui (contrato Q4-A1 §1): uma soltura cujo press o host não registrou encerra o drag ativo e não vai ao cliente (`InputManager.cpp:748–751`). Por isso, deixar passar a soltura de um press que a barra tomou é seguro: com drag ativo, o host o encerra pelo fim nativo; sem drag, a soltura não chega ao cliente.

## Goals / Non-Goals

**Goals:**
- nenhuma mudança de geometria ou foco pela barra enquanto houver drag ativo de qualquer origem;
- o drag da barra só começa na janela da barra;
- todo `grab` que não começa é erro Lua;
- decisões testáveis sem compositor, mortas por assertiva.

**Non-Goals:**
- mudar a política de fim (`alt5-rc` continua);
- mudar `hostDrag.hpp`, o núcleo Lua ou a UI;
- o achado 6 (spec × início no press para flutuante);
- reescrever os testes antigos da alpha-1 (achados 4 e 5).

## Decisions

### 1. Uma porta única antes do início

Nova decisão pura em `barInput.hpp`, chamada antes de qualquer mudança de geometria ou foco nas três rotas acima e no começo de `handleMovement`:

```
enum class eStartGate : uint8_t { GO, YIELD, NOT_OVER };
eStartGate startGate(bool dragActive, bool pointerOnOwnWindow);
// dragActive vence: YIELD; senão, fora da janela: NOT_OVER; senão GO.
```

- `dragActive` = `dragController()->target() != nullptr`, lido na hora.
- `pointerOnOwnWindow` = `pointerOverWindow()`, que já usa os mesmos flags de hit-test de `KwDrag::begin` (`RESERVED_EXTENTS | INPUT_EXTENTS | ALLOW_FLOATING`). Nada novo no host.
- O rearm por Shift já confere as duas coisas (`SDrag::onMotion`); passa a usar a mesma porta, sem mudar o comportamento.

### 2. YIELD: abortar sem tocar em nada

Com `YIELD`:
- o unsnap para onde está (`SUnsnap` ganha `abort()`, que só desliga `active`); a geometria não é mais alterada;
- o início pendente é limpo;
- foco e `begin` não são chamados;
- o par do botão esquerdo é liberado (`pairs.dragStarted(BTN_LEFT)`), para a soltura chegar ao compositor e encerrar o drag alheio pelo fim nativo, como manda o contrato (não cancelar soltura que encerraria drag alheio, O3).

A entrada de `restores` já apagada no início do unsnap não volta (ver Open Questions 1).

### 3. NOT_OVER: o press vira clique

Com `NOT_OVER` (sem drag ativo, ponteiro fora da janela da barra):
- sem foco nem `begin`, sem `m_drag` armado;
- o início pendente é limpo, então o mesmo press não tenta de novo;
- o par fica como está: a soltura é tomada pela barra, como num clique (não há drag a encerrar).

No fim do unsnap, a janela foi trazida para baixo do ponteiro, então o caso normal é `GO`. Se mesmo assim der `NOT_OVER`, o unsnap termina sem drag.

### 4. `grab`: todo início que não acontece é erro

Nova função pura em `barInput.hpp`:

```
std::string grabStartError(bool found, bool focused, bool started);
// "" só com os três verdadeiros; mensagem própria para janela não achada, foco recusado e início recusado (grabRefused()).
```

`luaGrab` passa a devolver `configError` com essa mensagem em todas as saídas sem drag, inclusive quando o laço termina sem achar a janela. O foco já mudado antes de um início recusado continua como hoje (o grab foca antes de iniciar, como em `bf2049a`).

## Risks / Trade-offs

- **Unsnap interrompido deixa a janela num tamanho intermediário.** É o preço de não mexer em geometria durante o drag alheio. Raro: exige que outro drag comece dentro dos 140 ms.
- **`NOT_OVER` num movimento rápido tira o arrasto do usuário.** Antes, a vizinha era arrastada (pior). Agora o press vira clique; o usuário aperta de novo.
- **Corrida entre a porta e `begin`:** os dois rodam no mesmo evento, sem entrada no meio. `begin` ainda confere `target()` depois das decorações (T9).
- O achado 1 é difícil de reproduzir ao vivo (outro drag dentro de 140 ms). A cobertura principal é host-free e estrutural; o roteiro ao vivo cobre o achado 2 e a regressão geral.

## Migration Plan

1. Aprovação do dono para esta proposta.
2. Implementação com autorização própria, vermelho primeiro.
3. Build e `check.sh` isolados.
4. Roteiro ao vivo do dono.
5. Arquivar e sincronizar os specs principais.

Voltar é reverter o commit da implementação.

## Validation Plan

| Requisito | Vermelho primeiro | Aceite e não-vacuidade |
|---|---|---|
| Porta | `startGate` com as 4 combinações de entrada | YIELD vence NOT_OVER; GO só com as duas condições |
| YIELD no unsnap | `SUnsnap::abort()` e passos depois dele | depois de abortar, nenhum passo muda a geometria; `active` desligado |
| Ordem nas rotas | inventário estrutural em `barDeco.cpp` | `startGate(` aparece antes de `applyUnsnap(`, `fullscreenWindow(`, `fullWindowFocus(` e `KwDrag::begin(` em cada rota; ignorando comentários |
| Soltura no YIELD | sequência press → YIELD → soltura com `SPairs` | a soltura não é tomada |
| Soltura no NOT_OVER | sequência press → NOT_OVER → soltura | a soltura é tomada (par de clique) |
| `grab` | `grabStartError` com as 8 combinações | "" só com tudo verdadeiro; cada falha tem mensagem; `luaGrab` sem `return 0` fora do caminho de sucesso (estrutural) |
| Regressão | `tools/test-plugin-input.sh` e `check.sh` | verdes |
| Ao vivo (dono) | — | lado a lado: press na barra e movimento rápido para a vizinha não arrasta a vizinha; drag, Shift, encaixe e unsnap normais continuam |

Mutações: o script novo roda primeiro um controle sem mutação (precisa passar), guarda a saída de cada mutante e só conta morto quando há uma linha `FALHOU -` que não seja de compilação. Mutantes mínimos: porta ignorando drag ativo; porta ignorando a janela; `abort()` sem efeito; YIELD sem liberar o par; `luaGrab` com `return 0` no fim do laço.

## Open Questions

1. **Restaurar a entrada de `restores` ao abortar o unsnap?** Decidido pelo dono em 01/11/2026: não (a janela já saiu do formato encaixado). Se o dono quiser que "restaurar" ainda funcione depois de um unsnap interrompido, a entrada é recolocada no `abort`.
2. **Mostrar o erro do `grab` no menu?** Hoje o núcleo engole o erro (`pcall`). Decidido pelo dono em 01/11/2026: fica fora desta mudança.
