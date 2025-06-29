# Sistema de Arquivos Invertido

Um sistema de arquivos FUSE que inverte nomes de arquivos e conteúdo. Quando você o monta, os nomes dos arquivos aparecem de trás para frente e o conteúdo é lido/escrito invertido.

## O que ele faz

- **Nomes de arquivos**: `test.txt` vira `tset.txt`
- **Conteúdo de arquivos**: `Hello World!` vira `!dlroW olleH`

## Requisitos

- Linux com suporte a FUSE
- libfuse3-dev ou libfuse-dev
- compilador g++

## Instalação

### Ubuntu/Debian
```bash
sudo apt install libfuse3-dev g++ make
```

### Fedora
```bash
sudo dnf install fuse3-devel gcc-c++ make
```

### Arch Linux
```bash
sudo pacman -S fuse3 gcc make
```

## Compilação

```bash
make
```

Ou use o script de compilação:
```bash
./compile.sh
```

## Uso

```bash
./reverse_fs <diretorio_real> <ponto_montagem>
```

### Exemplo

```bash
# Criar diretórios
mkdir -p /tmp/arquivos_reais
mkdir -p /tmp/montagem_invertida

# Criar alguns arquivos de teste
echo "Hello World!" > /tmp/arquivos_reais/test.txt
echo "12345" > /tmp/arquivos_reais/numbers.txt

# Montar o sistema de arquivos invertido
./reverse_fs /tmp/arquivos_reais /tmp/montagem_invertida

# Em outro terminal, verificar o sistema montado
ls -la /tmp/montagem_invertida
# Você verá: tset.txt, srebmun.txt

cat /tmp/montagem_invertida/tset.txt
# Você verá: !dlroW olleH

# Criar um novo arquivo no sistema montado
echo "oduetnoc ovoN " > /tmp/montagem_invertida/ovon.txt

# Verificar o diretório real
ls -la /tmp/arquivos_reais
# Você verá: novo.txt (nome invertido)

cat /tmp/arquivos_reais/novo.txt
# Você verá: Novo conteudo (conteúdo invertido)
```

## Desmontagem

```bash
fusermount -u <ponto_montagem>
```

## Como funciona

O sistema de arquivos intercepta todas as operações de arquivo e:
- Traduz caminhos virtuais para caminhos reais invertendo nomes de arquivos/diretórios
- Inverte o conteúdo dos arquivos ao ler/escrever
- Mantém as mesmas permissões e metadados dos arquivos