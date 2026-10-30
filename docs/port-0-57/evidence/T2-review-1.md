O diff de produção está correto e mínimo para W. Não encontrei blocker ou major no código. O parecer é **conditional** por problemas no oráculo e na documentação da evidência.

Revisão somente leitura: inspecionei fontes, scripts e logs; reexecutei apenas o comparador Python sobre os logs existentes. Não executei sondas, build ou teste ao vivo.

**Achados**

1. **minor — O runner pode contabilizar erro de compilação ou crash como mutação morta.**  
   Evidência: [T2-probe-run.sh:37](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/docs/port-0-57/evidence/T2-probe-run.sh:37) combina compilação e execução; a linha 39 aceita qualquer retorno diferente de zero. Não exige compilação bem-sucedida nem a assertiva esperada.  
   **Reprodução:** o próprio `T2-probe-run-red-HEAD.log` contém mutações marcadas `OK … killed` por membros ausentes no fake. O runner termina vermelho, portanto isso não invalida o vermelho global, mas demonstra o falso positivo local.  
   **Correção:** separar compilação de execução e exigir a mensagem específica da assertiva. No log verde apresentado, as oito mortes efetivamente mostram assertivas; o problema é a garantia oferecida pelo runner.

2. **minor — `T2_DTOR_NO_ERASE` testa uma propriedade estrutural, não uma diferença observável da implementação real.**  
   Evidência: [T2-hostfree.cpp:49](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/docs/port-0-57/evidence/T2-hostfree.cpp:49) e [T2-probe-run.sh:49](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/docs/port-0-57/evidence/T2-probe-run.sh:49).  
   **Reprodução:** executar MUT1 somente com o destrutor que faz `erase`: ela passa, conforme reconhecido em `T2-W-findings.md:51`. A publicação acontece depois da resposta do host e é desfeita pelo destrutor antes do retorno, sem observador intermediário nesse modelo.  
   A variante é **legítima para isolar a responsabilidade da guarda**. Deve ser contabilizada separadamente: sete mutações comportamentais detectadas com o destrutor representativo, mais uma verificação estrutural com rollback desativado.

3. **minor — A seção de implementação da evidência perdeu conteúdo.**  
   Evidência: [T2-W-findings.md:44](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/docs/port-0-57/evidence/T2-W-findings.md:44): há frases como “Diff: (só , +9/−7)” e referências/comandos vazios nas linhas seguintes. As linhas 24 e 29 também conservam formulações anteriores à decisão/implementação.  
   **Reprodução:** `sed -n '24,53p' docs/port-0-57/evidence/T2-W-findings.md`.  
   Restaurar símbolos, caminhos e comandos; esclarecer que atribuir `m_self` antes da chamada é intencional e não equivale a publicar em `bars`.

**Conclusões por item**

- **(a) Ordem e callbacks:** segura nas fontes examinadas. `WindowPresentation.cpp:97–118` insere a decoração antes de chamar `initializeAnimations`, `updateState`, `onWindowMap`, atualização e recálculo. Os três primeiros são no-ops herdados em `IHyprWindowDecoration.cpp:36–46`; `CHyprBar::updateWindow` apenas causa damage (`plugin/barDeco.cpp:1122`). Posicionamento e damage não procuram a barra em `bars`. Não identifiquei dependência da publicação antecipada. A lista do host já permite deduplicação durante os callbacks; o SP local mantém a barra viva, e `m_self`/`bars` são fracos.

- **(b) Caminho `false`:** `PluginAPI.cpp:143–157` retorna antes de mover o argumento nos casos de recusa. O temporário convertido libera sua referência; depois o SP local destrói a barra. O destrutor faz `erase` sem encontrar publicação. Os listeners são membros RAII; a animação é membro proprietário e desconecta-se da lista ativa ao morrer. Resta `m_scheduledRecalc = true` (`plugin/barDeco.cpp:57`): recálculo adicional, sem referência à barra recusada. É aceitável nesse caminho; não é rollback integral dos efeitos do construtor. O host-free não testa esses efeitos reais.

- **(c) Predicado:** conferido: mesmos sete átomos e override-redirect; Wayland mantém `suggestsNoBorder=false`. A pegajosidade antiga também está corretamente identificada. A frase “predicado no instante de `window.open` é o mesmo” deve ser entendida como **mesma expressão sobre o estado corrente**, ressalvado o histórico pegajoso já documentado. D1/D2 justificam a divergência aceita na opção A.

- **(d) Atualização:** sustenta-se no ponto de chamada normal. A busca integral encontrou apenas declaração, `push_back`, leitura e `clear` da fila. Entre o `push_back` e `updateDecorations`, `forceRecalcFor` apenas marca um booleano (`DecorationPositioner.cpp:100–106`). Não encontrei outro caminho normal que deixe a fila pendente. A ressalva de reentrância durante destruição/flush continua necessária; não se deve ampliar a conclusão para equivalência geral dos hosts.

- **(e) Provas:** a sonda confirma tipos e expressões contra headers reais; o vermelho de HEAD demonstra incompatibilidade com o contrato 0.57, não regressão comportamental executável. O host-free usa ownership real, mas substitui construtor, callbacks e host. As verificações textuais vinculam o código ao padrão esperado; não provam semântica de regras, Wayland ou X11 ao vivo.

- **(f) Build:** reproduzi a comparação dos logs: GCC **57→52**, Clang **65→60**. Saem cinco diagnósticos W; o diagnóstico de classe abstrata muda de `UniquePtr.hpp` para `SharedPtr.hpp`, mantendo a origem em `CHyprBar`. Sustenta **nenhum diagnóstico adicional além desse deslocamento**, não build funcional nem ausência de erros ainda mascarados.

**HANDOFF PARA PRÓXIMA SESSÃO**

Revisão W concluída; código recomendado para aprovação. Branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `dc4f48a312a7be99a2ae1b26a3166c734835c621`; alterações preexistentes em `plugin/main.cpp`, documentação e evidências preservadas. Nenhuma escrita ou commit realizado.

Opção A aprovada pelo dono; pendentes os três ajustes acima. Próxima ação recomendada: corrigir runner e registro documental sob autorização de escrita, depois conferir novamente a evidência. D/R, build completo e validação ao vivo continuam fora desta revisão.

**Veredito: conditional.**