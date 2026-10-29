# Settings: produto futuro e aplicação existente

Consolidação em 29/10/2026; baseline de código `a2506c6`. Este documento consolida D-23–D-27 e o detalhamento funcional proposto. Não afirma que o Settings completo da suíte já existe e não escolhe toolkit novo, serviços ou shell.

## Decisões e fronteiras

| Requisito | Aprovação de produto | Implementação e validação |
|---|---|---|
| Sistema permanente e Interface adaptativa ao rice | [D-23](DECISIONS.md#d-23) aprovada | Aplicação GTK4 atual não possui essa sidebar nem integração de rices; não testado |
| Padrão global de modo para novos workspaces, com exceção por monitor; modos existentes preservados | [D-24](DECISIONS.md#d-24) aprovada | Código persiste modo por workspace; padrão global/exceção por monitor ausentes |
| Perfis de monitores: manual por padrão, automação opcional após identidade/aplicação/recuperação confiáveis | [D-25](DECISIONS.md#d-25) aprovada | Gestor de perfis da suíte ausente; sem validação de aplicação ou rollback |
| Delegar política de idle às funções efetivamente fornecidas pelo rice; impedir concorrência | [D-26](DECISIONS.md#d-26) aprovada | Coordenação por rice ausente; serviços do desktop instalado não são prova de implementação Kwinish |
| Oferecer fallback compatível e verificado de idle/lock quando faltar no rice | [D-27](DECISIONS.md#d-27) aprovada | Fallback da suíte não implementado nem testado; ferramentas/handshake em aberto |

A [D-08](DECISIONS.md#d-08) mantém o gerenciador e configurações fundamentais independentes do rice. O [Settings atual](../../ui/src/settings.rs) em Rust/GTK4 edita opções do plugin e Snap Assist; os demais editores existentes continuam documentados na [API](../API.md) e em [CONFIGURAVEL](../CONFIGURAVEL.md). Reaproveitá-los é direção da fonte de produto, sujeita a inventário; não significa que estejam reunidos numa aplicação completa.

A consolidação documental realizou leitura estática e preservou registros anteriores. Não executou testes de UI, monitor, energia, bloqueio ou integração de rice. Evidências históricas da implementação ficam em [STATUS](../STATUS.md) e [TESTING](../TESTING.md), com seus limites.

## Arquitetura da informação proposta

```text
Sistema — permanece disponível com qualquer rice
  Janelas / Workspaces / Monitores / Energia e bateria
  Atalhos / Diagnóstico e recuperação
Interface — adapta-se ao rice
  Oficial: Barra / Dock / Launcher / Aparência
  Externo: Rice ativo / acesso ao configurador próprio, quando disponível
```

A D-23 aprova a adaptação e permanência; a hierarquia detalhada e cada controle não foram aprovados automaticamente. Som, rede, Bluetooth, entrada, aplicativos/inicialização, idioma/data e privacidade/segurança são expansões propostas. Busca na sidebar, agrupamentos e fases de implementação também continuam propostas.

## Requisitos funcionais recomendados

Todos os detalhamentos desta tabela são **propostas da fonte funcional**, sem implementação ou validação da suíte nesta tarefa. As decisões aprovadas que os motivam constam acima.

| Página | Detalhamento funcional e limites |
|---|---|
| Janelas | Ler modo e capacidades reais; selecionar alvo; reutilizar editores e ações existentes; distinguir floating manual de transição automática; expor opções somente com suporte comprovado |
| Workspaces | Inventário por monitor, modo efetivo e janelas; distinguir criação, restauração e reassociação; não tratar especiais internos como workspaces comuns. D-24 não altera modos já atribuídos |
| Monitores | Identidade real, modos, escala e posição; perfis para saídas presentes/conhecidas. Aplicação arriscada: snapshot, validar, aplicar temporariamente, confirmar e persistir ou reverter; mecanismo precisa existir antes de oferecer a operação |
| Energia e bateria | Separar perfil energético, idle, DPMS, lock, suspensão, hibernação e ações manuais; indicar responsável por função. Respeitar capacidades/inibidores; AC/bateria só onde aplicável |
| Atalhos | Reutilizar editor; autoria gerenciador/rice/usuário e conflitos observáveis. Atalhos privados sem API podem permanecer desconhecidos; `Super+Espaço` não é escolha aprovada |
| Diagnóstico | Componente operacional/incompatível/não carregado/falha/desconhecido com evidência; detectar conflito de idle/ausência de locker; backup e recuperação com efeitos claros, sem exigir o shell oficial |
| Interface oficial | Controles de barra/dock/launcher/aparência seguem [INTERFACE](INTERFACE.md) e [DESIGN](DESIGN.md), preservando preferências quando inativos |
| Rice externo | Mostrar identidade/capacidades verificáveis, acesso ao configurador real e retorno seguro; não oferecer controles fictícios nem escrever seus arquivos arbitrariamente |

## Responsabilidades técnicas propostas

| Domínio | Executor ou fonte a integrar | Limite |
|---|---|---|
| Janelas/workspaces/regras | Núcleo Lua e Hyprland | Settings observa e solicita ações; não mantém políticas concorrentes |
| Decorações/ações de título | Plugin C++ | Expor capacidades compatíveis e carregadas |
| Áudio / rede / Bluetooth / bateria | PipeWire/WirePlumber, NetworkManager, BlueZ, UPower, quando presentes | São candidatos de integração, não dependências novas aprovadas |
| Sessão e energia | Serviço de perfis e systemd-logind compatíveis | Exibir suporte, autorização, inibidores e resultado; não inferir sucesso pelo envio |
| Idle/DPMS/lock | Controlador efetivo por função e locker compatível | Um responsável por política; não implementar senha/autenticação próprias |
| Preferências do rice | Configurador do rice externo | Link/capacidades reais, sem presumir API inexistente |

Configuração atômica, backup, identificadores estáveis, IPC versionado com timeout e observação por eventos são recomendações técnicas; não são um contrato de integração já disponível. Cache deve ser invalidável e não substituir estado efetivo. A seleção de ferramenta de idle/lock, incluindo hypridle/hyprlock como candidatos, continua pendente.

## Inatividade e bloqueio

| Idle do rice verificado | Locker do rice verificado | Composição funcional recomendada | Implementação/validação |
|---|---|---|---|
| Sim | Sim | Delegar componentes do rice | Não implementada/testada pela suíte |
| Sim | Não | Idle externo + locker fallback, após validar ligação | Não implementada/testada |
| Não | Sim | Idle fallback + locker externo, após validar ligação | Não implementada/testada |
| Não | Não | Ambos da suíte, somente se disponíveis e verificados | Não implementada/testada |
| Desconhecido/falha | Qualquer | Diagnóstico e recuperação controlada; não iniciar rival às cegas | Política detalhada de falha pendente |

A responsabilidade é por capacidade: lock por idle, DPMS e suspensão automática podem ter proprietários diferentes. Comando de bloqueio emitido ou processo presente não comprova sessão bloqueada. A fonte recomenda confirmar lock antes de suspensão quando essa política for exigida, respeitar inibidores e não afirmar proteção após falha. Protocolo e política de falha requerem desenho e testes próprios.

O handoff proposto entre rices inventaria controladores, verifica capacidades, prepara um responsável por função, ativa sem concorrência e revalida com rollback. Aplicar inicialmente na próxima sessão é proposta, não decisão de ativação definitiva.

## Estados observáveis propostos

Controles relevantes distinguem **disponível**, **não suportado**, **desconhecido**, **delegado**, **salvo mas não aplicado**, **aplicando**, **aplicado confirmado** e **falha/conflito**. A gravação de arquivo e um subprocesso com saída zero não provam aplicação. Um estado agregado não pode esconder falha de bloqueio.

## Aceitação planejada — não executada

Os IDs originais ST-* são preservados para rastreabilidade. A aprovação de D-23–D-27 não aprova automaticamente os mecanismos nem torna estes testes executados.

| ID | Cenário e resultado a validar |
|---|---|
| ST-01 | Rice oficial: Sistema e Interface oficial acessíveis |
| ST-02 | Rice externo: Sistema persiste; Interface mostra Rice ativo |
| ST-03 | Troca e retorno preservam preferências oficiais |
| ST-04 | Novo workspace usa exceção de modo do monitor; antigos preservam modo |
| ST-05 | Hotplug/perfil manual mantém acesso utilizável |
| ST-06 | Configuração de monitor inválida falha ou reverte sem falsa confirmação |
| ST-07 | Rice com idle/locker: fallback não duplica políticas |
| ST-08 | Rice sem ambos: fallback só assume depois de verificação |
| ST-09 | Idle sem locker: fallback de lock integrado e validado |
| ST-10 | Locker sem idle: fallback de idle integrado e validado |
| ST-11 | Falha de lock antes de suspender não é registrada como proteção; política explícita |
| ST-12 | Crash/desconexão comunica degradação e recupera sem concorrência |
| ST-13 | Perfil energético alterado externamente aparece como estado real |
| ST-14 | Falha Lua/Hyprland IPC mantém UI e diagnóstico utilizáveis |
| ST-15 | Dados inválidos/backup: sem corrupção e com recuperação |
| ST-16 | Rice sem configurador: nenhum acesso fictício |
| ST-17 | Inibidor de jogos/mídia respeitado onde suportado |
| ST-18 | Lock Wayland efetivo, além de comando/processo |

Fases sugeridas, extensões e escolhas de serviços permanecem em [PROPOSALS](PROPOSALS.md). Não há compromisso de roadmap nesta consolidação.

## Fontes preservadas

- [Settings consolidado v1.0, §§1–11](../history/product-import/2026-10-28/external-originals/docs/product/features/SETTINGS_ARQUITETURA_FUNCIONAL_CONSOLIDADA_v1.0.md).
- [Visão v3.13, §§23–28](../history/product-import/2026-10-28/external-originals/docs/product/vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md).
- [Registro das decisões](DECISIONS.md) e [manifesto de fontes](../history/product-import/2026-10-28/SOURCES.json).
