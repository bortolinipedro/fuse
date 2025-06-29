#!/bin/bash

# Script para testar o sistema de arquivos invertido

set -e

echo "=== Teste do Sistema de Arquivos Invertido FUSE ==="

# Cria diretórios de teste
REAL_DIR="/tmp/reverse_test_real"
MOUNT_DIR="/tmp/reverse_test_mount"

echo "Criando diretórios de teste..."
mkdir -p "$REAL_DIR"
mkdir -p "$MOUNT_DIR"

# Cria alguns arquivos de teste no diretório real
echo "Criando arquivos de teste..."
echo "Hello World!" > "$REAL_DIR/test.txt"
echo "FUSE é muito legal!" > "$REAL_DIR/exemplo.txt"
echo "12345" > "$REAL_DIR/numeros.txt"

echo "Conteúdo do diretório real ANTES da montagem:"
ls -la "$REAL_DIR"
echo

echo "Conteúdo dos arquivos reais:"
for file in "$REAL_DIR"/*; do
    if [ -f "$file" ]; then
        echo "--- $(basename "$file") ---"
        cat "$file"
        echo
    fi
done

echo "Compilando o sistema de arquivos..."
make clean
make

echo "Montando o sistema de arquivos invertido..."
echo "Comando: ./reverse_fs '$REAL_DIR' '$MOUNT_DIR'"

# Monta o sistema em background
./reverse_fs "$REAL_DIR" "$MOUNT_DIR" &
FUSE_PID=$!

# Aguarda um momento para a montagem
sleep 2

echo "Sistema montado! PID: $FUSE_PID"
echo

echo "Conteúdo do diretório virtual (nomes invertidos):"
ls -la "$MOUNT_DIR"
echo

echo "Conteúdo dos arquivos virtuais (conteúdo invertido):"
for file in "$MOUNT_DIR"/*; do
    if [ -f "$file" ]; then
        echo "--- $(basename "$file") (nome invertido) ---"
        cat "$file"
        echo
    fi
done

echo "Testando criação de novo arquivo no sistema virtual..."
echo "Novo arquivo teste" > "$MOUNT_DIR/oven.txt"

echo "Verificando se o arquivo foi criado no diretório real com nome invertido:"
ls -la "$REAL_DIR"
echo

if [ -f "$REAL_DIR/nevo.txt" ]; then
    echo "Conteúdo do arquivo real 'nevo.txt' (deveria estar invertido):"
    cat "$REAL_DIR/nevo.txt"
    echo
fi

echo "Pressione ENTER para desmontar o sistema..."
read

echo "Desmontando..."
kill $FUSE_PID 2>/dev/null || true
fusermount -u "$MOUNT_DIR" 2>/dev/null || true

echo "Limpando diretórios de teste..."
rm -rf "$REAL_DIR" "$MOUNT_DIR"

echo "Teste concluído!"