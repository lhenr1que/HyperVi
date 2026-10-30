O diff T1 é correto e mínimo. Não encontrei **blocker** ou **major** no código de produção. Há limitações menores nas provas e uma referência documental desatualizada.

**(a) Semântica e escopo**

- Em `plugin/barDeco.cpp:486` e `:605`, mudou somente a qualificação de Shift. O bit continua sendo `1 << 0`: referência 0.56.2 em `src/devices/IKeyboard.hpp:14`; header instalado em `/usr/include/hyprland/src/input/Keys.hpp:10`. A chamada a `getModsFromAllKBs()` permanece igual, preservando a consulta ao estado agregado fornecido pelo compositor.
- **Sim, com configuração Lua na 0.56.2 os sete registros já ocorriam.** Em `HEAD:plugin/main.cpp:436–447`, `CONFIG_LUA` entrava no `else` de `add_button` e também satisfazia `type() != CONFIG_LEGACY`, registrando as outras seis funções. A sequência e os callbacks foram preservados em `plugin/main.cpp:436–442`.
- Foi removido somente o registro do keyword legado. `onNewButton` permanece definido, com ligação externa, em `plugin/main.cpp:106`.

Reprodução: `git diff -- plugin/` e `git show HEAD:plugin/main.cpp`. O diff local coincide com o apresentado.

**(b) Operação prevista pelo header**

A expressão usa diretamente a API fornecida pelo Hyprland:

- `/usr/include/hyprland/src/macros/Enums.hpp:9`: conversão implícita do enum para a máscara;
- `:36`: `operator&(name, name)`, devolvendo máscara;
- `:15`: `explicit operator bool()`, válido na condição de `if`.

Não há wrapper inventado na produção. `kwShift` é apenas uma função da sonda.

**(c) Sonda e runner**

Reexecutei os **18 casos de compilação**, sem gerar arquivos: positivo, três negativos e cinco mutações, com GCC e Clang. Ambos os positivos retornaram **0**; os negativos e mutantes retornaram **1**, com os diagnósticos esperados. As cinco mutações falharam especificamente na assertiva de equivalência de `T1-probe.cpp:63`.

Não executei literalmente o runner, pois ele cria um diretório temporário; reproduzi suas compilações com `-fsyntax-only`.

Achados:

1. **minor — Oráculo desacoplado da produção.**  
   Evidência: `docs/port-0-57/evidence/T1-probe.cpp:33–66` e `:69–72`. A equivalência exaustiva cobre a expressão copiada; `prodShape` também é uma cópia. Nenhum trecho extrai os dois testes reais ou verifica os sete registros Lua.  
   **Como reproduzir:** numa cópia descartável, trocar Shift por Ctrl somente em `barDeco.cpp:605`, ou remover um registro de `main.cpp`: a sonda permaneceria verde.  
   Portanto, prova a equivalência das expressões para as 256 máscaras, mas não constitui teste de regressão diretamente ligado aos arquivos de produção, nem teste de múltiplos teclados reais.

2. **minor — Runner aceita diagnósticos pouco específicos.**  
   Evidência: `T1-probe-run.sh:14` e `:19–22`. Basta saída diferente de zero e ocorrência do padrão em qualquer parte do diagnóstico.  
   **Como reproduzir:** numa cópia da sonda, inserir um `#error static_assert` condicionado a `PROBE_MUT`: os mutantes seriam aceitos como detectados mesmo sem falhar pelo oráculo esperado.  
   Isso não invalida os resultados atuais: na minha reprodução, todos falharam pela assertiva correta.

Os negativos demonstram incompatibilidade das formas antigas com os headers atuais; não demonstram, sozinhos, a cronologia “teste vermelho antes da edição”.

**(d) Comparação de build**

Os resultados apresentados sustentam a afirmação restrita: **“zero diagnósticos `error:` novos capturados pelo comparador”**. As quatro remoções são coerentes com T1. Aparecem três chaves porque a chave de Shift tem multiplicidade dois.

3. **minor — Comparador não garante ausência geral de erros novos.**  
   Evidência: script fornecido, `src()` linha 5 e regex de `keys()` linha 11. Caminhos absolutos são lidos do disco mesmo para a baseline; `fatal error:`, erros de linker e outros formatos ficam fora da contagem. O texto da linha também perde a identidade entre ocorrências iguais.  
   **Como reproduzir:** fornecer um log com `fatal error:` adicional: ele não altera o contador; usar caminhos absolutos da baseline após modificar o fonte faz a associação usar o conteúdo atual.

`-ferror-limit=0` resolve o limite de diagnósticos do Clang, mas não essas limitações. Não localizei os logs T1 brutos nas evidências consultadas; **não confirmei independentemente os totais 61→57 e 69→65**, nem repeti o build completo. Não há fundamento para declarar build bem-sucedido ou compatibilidade funcional.

**(e) Referências restantes**

4. **minor — Ledger ainda descreve o registro legado como existente.**  
   Evidência: `docs/port-0-57/LEDGER.md:33`, item L05. A descrição e as linhas citadas ficaram desatualizadas após T1.  
   **Como reproduzir:** comparar essa linha com `plugin/main.cpp:436–442`.

Também encontrei `CONFIG_LEGACY` e `HL_MODIFIER_SHIFT` em `P5-WINDOW-API.md:74` e `P6-LOG-API.md:58`; são registros históricos de erros persistentes naquelas etapas, não defeitos atuais a corrigir retroativamente.

A busca em `core`, `tools`, `ui`, `recipes`, OpenSpec, API e CONFIGURAVEL não encontrou consumidores restantes desses símbolos ou do keyword exato. `plugin:kwinish:button_hover_color` é outra opção e permanece válido.

**HANDOFF PARA PRÓXIMA SESSÃO**

Revisão T1 concluída; produção aprovada no escopo fechado. Branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `f76c785`; alterações e arquivos não rastreados preexistentes preservados. Nenhuma escrita, commit, build completo ou ação no compositor. Este relatório deve ser incorporado posteriormente ao registro de continuidade, quando houver autorização de escrita. Próxima ação recomendada: registrar as limitações das provas e atualizar L05; qualquer implementação adicional exige nova autorização.

Veredito: **approve**.