# Kwinish beta: o que falta para lançar (rascunho, 26/10/2026)

Lançar como produto à parte, beta, para Arch + Hyprland 0.56.x. Sem depender do Ryoku nem do HDE.

## Bloqueia o beta
1. **Dono verifica ao vivo** os testes pendentes de `STATUS.md` (plugin recompilado, Snap Assist com Shift).
2. **Bug do encaixe entre monitores** (volta ao monitor de origem): corrigir ou listar como limitação conhecida.
3. **Instalação limpa:** `kwinish-git` + `kwinish-bars-plugin-git` instalam e rodam numa máquina/VM Arch sem Ryoku; hook do pacman testado num upgrade do `hyprland`; remover o pacote não deixa lixo.
4. **Nome definitivo** (hoje "Kwinish") e checagem de que não conflita com marca existente.
5. **Licenças:** `LICENSE` (MIT), `plugin/LICENSE.hyprland-plugins` (BSD-3), cabeçalhos SPDX; README com créditos (hyprbars; Omarchy como referência, sem código copiado).
6. **Docs públicos:** README, `PLUGIN.md`, `API.md`, "limitações conhecidas", como reportar bug. Docs internos (HANDOFF, spike/, referencia/) ficam fora do repositório público ou numa pasta `internal/`.
7. **Repositório público:** hoje só existe local. Remover do histórico qualquer dado pessoal (e-mail, caminhos) antes do primeiro push; decisão do dono.

## Não bloqueia (vira roadmap)
Duplo clique no ícone fecha, miniaturas no Alt+Tab, modais ao minimizar, seletor ao passar no maximizar, indicador de modo, escala fracionária e monitores girados, auto-raise.

## Declarar como limitações no beta
Só Hyprland 0.56.x (plugin recompila a cada versão; modo de emergência com barras de reserva); janelas ladrilhadas ignoram snap/zonas; um monitor testado a fundo.

## Critério de pronto
Testes (`core/test_*.lua`, `cargo test`) verdes, checklist 1 a 7 fechado, uma pessoa que não é o dono instala só pelo README.
