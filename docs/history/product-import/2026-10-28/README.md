# Importação documental de 28/10/2026

Consolidada em 29/10/2026 com autorização exclusivamente documental. Baseline do repositório: `a2506c63dad16384414dc95919b8292aead284df`.

## Acervo

- [Pacote externo preservado](Kwinish_Product_Docs_2026-10-28.zip): cópia byte a byte do ZIP que também permanece na raiz.
- [Originais do repositório](repository-originals.zip): os 28 documentos foram arquivados antes de qualquer substituição; nomes das entradas preservam seus caminhos de origem.
- [Entradas externas](external-originals/): as 31 entradas do ZIP, extraídas sem alterações para consulta e proveniência.
- [SOURCES.json](SOURCES.json): inventário inicial, hashes de origem e entradas dos arquivos.
- [MANIFEST.md](MANIFEST.md) e [MANIFEST.json](MANIFEST.json): correspondência de cada origem com preservação e destinos de leitura.
- [SHA256SUMS](SHA256SUMS): integridade dos originais e cópias preservadas; caminhos relativos à raiz do repositório.
- [PROTECTED.json](PROTECTED.json): estado Git inicial, hashes dos arquivos protegidos e metadados das worktrees.
- [VALIDATION.md](VALIDATION.md) e [VALIDATION.json](VALIDATION.json): resultado da revisão documental, links e divergências.

Os originais conservam versões internas inconsistentes, prompts e links históricos. Consulte [índice documental](../../../README.md) e [produto consolidado](../../../product/README.md) para leitura vigente.

## Verificar integridade

A partir da raiz do Kwinish:

```sh
sha256sum -c docs/history/product-import/2026-10-28/SHA256SUMS
```

Para conferir também cada entrada original guardada no ZIP do repositório e no pacote externo, sem extrair nem modificar arquivos:

```sh
python3 - <<'PY'
from pathlib import Path
import hashlib, json, zipfile
root = Path.cwd()
base = root / 'docs/history/product-import/2026-10-28'
sources = json.loads((base / 'SOURCES.json').read_text())
digest = lambda data: hashlib.sha256(data).hexdigest()
with zipfile.ZipFile(root / sources['repository_originals_archive']['path']) as archive:
    for item in sources['repository_originals']:
        assert digest(archive.read(item['archive_entry'])) == item['sha256'], item['path']
with zipfile.ZipFile(root / sources['external_package']['preserved_path']) as archive:
    for item in sources['external_entries']:
        assert digest(archive.read(item['archive_path'])) == item['sha256'], item['archive_path']
        assert digest((root / item['preserved_path']).read_bytes()) == item['sha256'], item['preserved_path']
print('28 originais do repositório e 31 entradas externas íntegros')
PY
```

Esses comandos verificam dados; não executam software contido nos pacotes. Os hashes de documentos consolidados pertencem ao manifesto da consolidação; documentos em manutenção poderão mudar posteriormente. O acervo original deve permanecer imutável.
