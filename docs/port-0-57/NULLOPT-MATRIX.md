# Matriz de tratamento de `std::nullopt` pelos futuros consumidores (investigação; nada implementado)

Preparatório, 30/10/2026. `geometry.hpp` devolve `std::nullopt` para entrada ou saída fora do contrato ([GEOMETRY.md](GEOMETRY.md) §4). Esta matriz propõe como cada consumidor deve reagir quando a **integração** (tarefas 4.2/4.4, **não autorizada**) existir. Nenhum consumidor foi alterado; os comportamentos abaixo são **propostas**, não decisões do dono, e nenhuma é validada contra o compositor.

## Princípio comum

1. **Uma validade por quadro, não por consumidor.** Se cada consumidor chamar a função e tratar `nullopt` à sua maneira, a barra pode ser desenhada mas não clicável (ou o inverso). Proposta: o snapshot de apresentação do quadro (G2, `SPresentedGeometry`) guarda o resultado calculado **uma vez** (`optional` ou flag `valid`) e todos os consumidores o leem.
2. **`nullopt` nunca vira caixa zero.** Proibido `value_or({})`, `.value()` sem teste e `*opt` sem teste: um box zero desenharia na origem e geraria damage vazio.
3. **Falha segura = não desenhar, não aceitar input, mas sempre invalidar o que já estava na tela.** Damage deve cobrir a última caixa válida (senão sobra pixel velho).
4. **Registro limitado:** um log por transição válido→inválido (não por quadro).

## Matriz

| Consumidor | Funções | Comportamento seguro proposto | Risco de inconsistência | Teste necessário |
|---|---|---|---|---|
| **Desenho** (`draw`, `titleBarBox`) | `physicalLocal`, `logicalBBox` | Não enfileirar `CBarPassElement` neste quadro; sem desenho parcial; nenhum GL tocado (preserva as saídas antecipadas de GL-STATE §4) | Barra sumir enquanto o input ainda aceita clique; piscar se alternar válido/inválido por 1 quadro | Host-free: snapshot fora do domínio ⇒ 0 elementos enfileirados, contador de passes (Q1) inalterado. Aninhado: janela com origem >2^31 não desenha nem trava |
| **Damage** (`damageEntire`, `damageBox`) | `logicalBBox`, `unionDamage` | Nova caixa `nullopt`: danificar a **caixa anterior** (se havia) e, se nem ela existe ou a união deu `nullopt` (maior que `kMaxBox`), danificar o **monitor inteiro**. Nunca omitir damage | Pixels residuais da barra antiga; custo de redraw total se o estado inválido persistir | Host-free: stub do consumidor com 4 casos (ambos válidos, novo inválido, anterior ausente, união estourada) ⇒ região danificada ⊇ último desenho. Aninhado D14/D15: damage parcial = redraw completo |
| **Input** (`cursorRelativeToBar`, hit-test, drag) | `logicalBBox`/snapshot | Sem caixa válida ⇒ **sem hit**: o evento passa para a janela; `cursorRelativeToBar` devolve vazio (não 0,0). Drag em curso que fica inválido ⇒ cancelar o drag e restaurar o modo de mouse-bind (Q4) | Clique "fantasma" numa barra invisível, ou clique perdido numa visível (se o snapshot do input for de outro quadro que o do desenho) | Host-free: hit-test com snapshot inválido ⇒ nenhum hit; transição válido→inválido durante drag ⇒ estado limpo. Aninhado D16/D17: botões/touch sem transformação reaplicada |
| **Preview** (`drawSnapPreview`, `CPreviewPassElement`) | `previewBBox` (entrada física) | `nullopt` ⇒ não enfileirar o elemento e não devolver `boundingBox`; danificar o preview anterior. **Decisão do dono pendente:** a ação de snap ao soltar continua pela lógica de zona (independe do desenho) ou é cancelada? Proposta: continua, só não desenha | Soltar a janela num snap que o usuário não viu; z-order G4 | Host-free: caixa colapsada/absorvida e escala fora de [1/16,16] ⇒ nenhum elemento, damage do anterior. Aninhado D12: comparação de capturas |
| **Tooltip** (`BAR = assignedBoxGlobal()`) | snapshot da barra | Barra sem geometria válida ⇒ esconder tooltip armado/visível (cancela timer, Q4); nunca ancorar na origem | Tooltip flutuando sem barra; timer armado após o unload (B1) | Host-free (`tooltip.hpp`): âncora inexistente ⇒ estado `hidden`. Aninhado D2/D3 e política G5 de captura |

## Casos transversais a testar na integração

- **Transição** válido→inválido→válido: sem vazamento de estado (passes, timers, drag) e sem damage perdido.
- **Rejeição por entrada vs. por saída:** `unionDamage`/`previewBBox` rejeitam também o **resultado** (largura >`kMaxBox`, extensão colapsada); o consumidor trata ambos igual (fallback de damage total).
- **Fonte dos limites:** quando o consumidor registrar o motivo, usar texto genérico ("fora do contrato geométrico"), sem afirmar que o Hyprland nunca fornece tais valores (H-DOM é contrato interno).
- **Contagem em produção:** expor, ao menos nos testes, quantas vezes cada consumidor caiu no ramo `nullopt` (a integração deve provar que o ramo é raro; se ocorrer com valores reais, o domínio H-DOM precisa ser revisto).
- **Lint de integração:** busca por `value_or`, `.value()` e `*` sobre os resultados das quatro funções deve dar 0 ocorrências fora dos testes.

## Pendências de decisão (dono)

G2 (snapshot por quadro), comportamento do snap sem preview, política de damage total como fallback e se o ramo `nullopt` deve avisar o usuário ou só registrar log.
