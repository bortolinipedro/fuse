#!/bin/bash

echo "Compilando reverse_fs..."

# Remove executável anterior
rm -f reverse_fs

# Tenta compilar com fuse3 primeiro
echo "Tentando com libfuse3..."
if g++ -Wall -Wextra -std=c++11 -D_FILE_OFFSET_BITS=64 -o reverse_fs reverse_fs.cpp -lfuse3 2>/dev/null; then
    echo "✓ Sucesso com libfuse3!"
else
    echo "Tentando com libfuse2..."
    if g++ -Wall -Wextra -std=c++11 -D_FILE_OFFSET_BITS=64 -o reverse_fs reverse_fs.cpp -lfuse 2>/dev/null; then
        echo "✓ Sucesso com libfuse2!"
    else
        echo "❌ Erro na compilação!"
        echo "Instale as dependências:"
        echo "Ubuntu/Debian: sudo apt install libfuse3-dev g++"
        echo "Fedora: sudo dnf install fuse3-devel gcc-c++"
        exit 1
    fi
fi

echo "Executável 'reverse_fs' criado!"
echo "Uso: ./reverse_fs <dir_real> <ponto_montagem>"