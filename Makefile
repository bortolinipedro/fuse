CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -D_FILE_OFFSET_BITS=64
LIBS = -lfuse3

# Se não tiver libfuse3, tente com libfuse (versão 2.x)
# LIBS = -lfuse

TARGET = reverse_fs
SOURCE = reverse_fs.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)

clean:
	rm -f $(TARGET)

install-deps-ubuntu:
	sudo apt-get update
	sudo apt-get install libfuse3-dev pkg-config

install-deps-fedora:
	sudo dnf install fuse3-devel

install-deps-arch:
	sudo pacman -S fuse3

.PHONY: all clean install-deps-ubuntu install-deps-fedora install-deps-arch