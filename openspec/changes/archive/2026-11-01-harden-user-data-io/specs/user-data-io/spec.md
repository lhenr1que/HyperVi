# Spec Delta

## Purpose

Proteger os arquivos locais de configuração e de comunicação do Kwinish contra execução de conteúdo, interpretação ambígua e publicação de dados parciais, preservando formatos válidos e arquivos anteriores em falhas.

## ADDED Requirements

### Requirement: Conteúdo JSON nunca executa código
O sistema SHALL interpretar arquivos JSON exclusivamente como dados e rejeitar conteúdo sintaticamente inválido sem bloquear a sessão. As leituras SHALL limitar tamanho a 1 MiB e profundidade a 32 níveis, com falha controlada ao exceder esses limites.

#### Scenario: Expressão Lua num arquivo
- **WHEN** um arquivo de regras ou layouts contém uma função Lua com laço infinito
- **THEN** a leitura termina com resultado vazio/erro controlado, nenhum código do arquivo é executado e a sessão continua responsiva

#### Scenario: Limites de leitura
- **WHEN** um arquivo excede 1 MiB ou 32 níveis de aninhamento
- **THEN** o leitor recusa o conteúdo antes de processá-lo integralmente e não aplica entradas extraídas por busca textual

#### Scenario: Leitura pela interface
- **WHEN** `kwinish-ui` lê um arquivo de configuração ou runtime acima dos limites
- **THEN** trata o conteúdo como inválido, como o núcleo, e uma gravação posterior preserva o original em backup antes de substituí-lo

### Requirement: Estrutura dos documentos é respeitada
O sistema SHALL aplicar somente entradas no nível e tipo de raiz definidos pelo contrato de cada arquivo. Erros de sintaxe ou tipo de raiz invalidam o documento; entradas semanticamente inválidas num documento válido SHALL ser descartadas individualmente. Campos desconhecidos SHALL ser ignorados.

#### Scenario: Nome conhecido em objeto desconhecido
- **WHEN** settings.json contém `{"unknown":{"enabled":false},"enabled":true}`
- **THEN** o valor aplicado para enabled é true

#### Scenario: Compatibilidade de strings e números
- **WHEN** um documento válido usa strings escapadas, Unicode ou números JSON válidos
- **THEN** o leitor os decodifica como dados e o validador de domínio aplica os mesmos tipos exigidos pelo contrato, sem confundir strings com booleanos ou números

#### Scenario: Documento truncado
- **WHEN** um documento contém uma entrada aparentemente válida mas termina com sintaxe incompleta
- **THEN** nenhuma entrada desse documento é aplicada

#### Scenario: Interface mostra o que o núcleo aplica
- **WHEN** rules.json traz `{"kitty":{"above":"yes"}}` ou workspace-modes.json traz um modo desconhecido
- **THEN** o menu da janela não marca a regra nem o modo, como o núcleo não os aplica

### Requirement: Layouts têm os mesmos limites no núcleo e na interface
O sistema SHALL usar validação equivalente em Lua e Rust para os layouts do contrato existente. Gap inteiro fora de 0–60 SHALL ser limitado a esse intervalo; gap ausente ou de tipo inválido SHALL usar o padrão. Nenhuma zona SHALL ser publicada com largura ou altura não positiva ou coordenadas não finitas.

#### Scenario: Espaçamento muito grande
- **WHEN** um layout contém gap igual a 999999
- **THEN** núcleo e interface interpretam gap como 60, sem dimensões negativas publicadas

#### Scenario: Zona menor que o espaçamento
- **WHEN** uma zona válida em frações produziria dimensões não positivas após aplicar o espaçamento na área útil
- **THEN** ela não é publicada nem enviada como geometria de janela inválida

### Requirement: Publicação preserva a versão anterior em falhas
O sistema SHALL publicar cada arquivo de configuração ou estado por substituição atômica no mesmo diretório. Falhas de serialização, escrita, fechamento ou rename SHALL preservar os bytes anteriores no destino, ser reportadas ao chamador e impedir anúncio de sucesso. Leitores concorrentes SHALL observar apenas a versão anterior ou a nova completa.

#### Scenario: Erro antes da publicação
- **WHEN** uma gravação existente sofre falha de escrita, fechamento ou rename
- **THEN** o arquivo anterior permanece intacto, o chamador recebe falha e o temporário é removido quando possível

#### Scenario: Primeira gravação falha
- **WHEN** ainda não existe destino e a escrita falha
- **THEN** nenhum arquivo parcial aparece como destino

### Requirement: Arquivo inválido só é substituído com backup preservado
O sistema SHALL preservar uma cópia exata do arquivo de configuração inválido antes de substituí-lo. Se não conseguir ler o arquivo ou concluir o backup, SHALL abortar a substituição com erro. Essa exigência não se aplica aos arquivos runtime descartáveis.

#### Scenario: Backup falha
- **WHEN** uma configuração inválida existe e a cópia de backup falha
- **THEN** a gravação é recusada e a configuração original permanece intacta
