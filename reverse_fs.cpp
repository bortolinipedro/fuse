#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>

// Diretório base onde os arquivos reais estão armazenados
static std::string root_path;

// Função para inverter uma string
std::string reverse_string(const std::string& str) {
    std::string reversed = str;
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

// Função para obter o caminho real a partir do caminho virtual invertido
std::string get_real_path(const char* path) {
    std::string virtual_path(path);
    
    // Se for root, retorna o diretório base
    if (virtual_path == "/") {
        return root_path;
    }
    
    // Remove a barra inicial
    if (virtual_path[0] == '/') {
        virtual_path = virtual_path.substr(1);
    }
    
    // Inverte o nome do arquivo/diretório
    std::string reversed_name = reverse_string(virtual_path);
    
    return root_path + "/" + reversed_name;
}

// Função para obter o nome virtual a partir do nome real
std::string get_virtual_name(const std::string& real_name) {
    return reverse_string(real_name);
}

static int reverse_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    std::string real_path = get_real_path(path);
    
    int res = lstat(real_path.c_str(), stbuf);
    if (res == -1) {
        return -errno;
    }
    
    return 0;
}

static int reverse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;
    
    std::string real_path = get_real_path(path);
    DIR *dp = opendir(real_path.c_str());
    
    if (dp == NULL) {
        return -errno;
    }
    
    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        // Pula . e ..
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            filler(buf, de->d_name, NULL, 0, static_cast<fuse_fill_dir_flags>(0));
            continue;
        }
        
        // Inverte o nome do arquivo para mostrar no sistema virtual
        std::string virtual_name = get_virtual_name(de->d_name);
        
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        
        if (filler(buf, virtual_name.c_str(), &st, 0, static_cast<fuse_fill_dir_flags>(0))) {
            break;
        }
    }
    
    closedir(dp);
    return 0;
}

static int reverse_open(const char *path, struct fuse_file_info *fi) {
    std::string real_path = get_real_path(path);
    
    int fd = open(real_path.c_str(), fi->flags);
    if (fd == -1) {
        return -errno;
    }
    
    fi->fh = fd;
    return 0;
}

static int reverse_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    (void) path;
    
    int fd = fi->fh;
    
    // Obtém o tamanho total do arquivo
    struct stat st;
    if (fstat(fd, &st) == -1) {
        return -errno;
    }
    
    size_t file_size = st.st_size;
    
    // Calcula o offset invertido
    off_t real_offset = file_size - offset - size;
    if (real_offset < 0) {
        real_offset = 0;
        size = file_size - offset;
    }
    
    if (offset >= file_size) {
        return 0;
    }
    
    // Lê os dados do arquivo
    std::vector<char> temp_buf(size);
    int res = pread(fd, temp_buf.data(), size, real_offset);
    
    if (res == -1) {
        return -errno;
    }
    
    // Inverte os dados lidos
    std::reverse(temp_buf.begin(), temp_buf.begin() + res);
    memcpy(buf, temp_buf.data(), res);
    
    return res;
}

static int reverse_write(const char *path, const char *buf, size_t size,
                        off_t offset, struct fuse_file_info *fi) {
    (void) path;
    
    int fd = fi->fh;
    
    // Obtém o tamanho atual do arquivo
    struct stat st;
    if (fstat(fd, &st) == -1) {
        return -errno;
    }
    
    size_t file_size = st.st_size;
    
    // Inverte os dados a serem escritos
    std::vector<char> reversed_buf(buf, buf + size);
    std::reverse(reversed_buf.begin(), reversed_buf.end());
    
    // Calcula o offset invertido
    off_t real_offset = file_size - offset - size;
    if (real_offset < 0) {
        // Se o offset invertido for negativo, estende o arquivo
        if (ftruncate(fd, file_size + (-real_offset)) == -1) {
            return -errno;
        }
        real_offset = 0;
    }
    
    int res = pwrite(fd, reversed_buf.data(), size, real_offset);
    if (res == -1) {
        return -errno;
    }
    
    return res;
}

static int reverse_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    std::string real_path = get_real_path(path);
    
    int fd = creat(real_path.c_str(), mode);
    if (fd == -1) {
        return -errno;
    }
    
    fi->fh = fd;
    return 0;
}

static int reverse_unlink(const char *path) {
    std::string real_path = get_real_path(path);
    
    int res = unlink(real_path.c_str());
    if (res == -1) {
        return -errno;
    }
    
    return 0;
}

static int reverse_mkdir(const char *path, mode_t mode) {
    std::string real_path = get_real_path(path);
    
    int res = mkdir(real_path.c_str(), mode);
    if (res == -1) {
        return -errno;
    }
    
    return 0;
}

static int reverse_rmdir(const char *path) {
    std::string real_path = get_real_path(path);
    
    int res = rmdir(real_path.c_str());
    if (res == -1) {
        return -errno;
    }
    
    return 0;
}

static int reverse_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
    std::string real_path = get_real_path(path);
    
    int res;
    if (fi != NULL) {
        res = ftruncate(fi->fh, size);
    } else {
        res = truncate(real_path.c_str(), size);
    }
    
    if (res == -1) {
        return -errno;
    }
    
    return 0;
}

static int reverse_release(const char *path, struct fuse_file_info *fi) {
    (void) path;
    
    close(fi->fh);
    return 0;
}

static int reverse_access(const char *path, int mask) {
    std::string real_path = get_real_path(path);
    
    int res = access(real_path.c_str(), mask);
    if (res == -1) {
        return -errno;
    }
    
    return 0;
}

static struct fuse_operations reverse_oper;

void init_fuse_operations() {
    memset(&reverse_oper, 0, sizeof(reverse_oper));
    
    reverse_oper.getattr = reverse_getattr;
    reverse_oper.readdir = reverse_readdir;
    reverse_oper.open = reverse_open;    
    reverse_oper.read = reverse_read;
    reverse_oper.write = reverse_write;
    reverse_oper.release = reverse_release;
    reverse_oper.create = reverse_create;
    reverse_oper.unlink = reverse_unlink;
    reverse_oper.mkdir = reverse_mkdir;
    reverse_oper.rmdir = reverse_rmdir;
    reverse_oper.truncate = reverse_truncate;
    reverse_oper.access = reverse_access;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <diretório_real> <ponto_montagem> [opções_fuse]" << std::endl;
        std::cerr << "Exemplo: " << argv[0] << " /tmp/real_files /tmp/reverse_fs" << std::endl;
        return 1;
    }
    
    // Define o diretório raiz onde os arquivos reais estão
    root_path = argv[1];
    
    // Remove o primeiro argumento (diretório real) dos argumentos do FUSE
    for (int i = 1; i < argc - 1; i++) {
        argv[i] = argv[i + 1];
    }
    argc--;
    
    // Inicializa a estrutura de operações FUSE
    init_fuse_operations();
    
    std::cout << "Montando sistema de arquivos invertido..." << std::endl;
    std::cout << "Diretório real: " << root_path << std::endl;
    std::cout << "Ponto de montagem: " << argv[1] << std::endl;
    std::cout << "Para desmontar: fusermount -u " << argv[1] << std::endl;
    
    return fuse_main(argc, argv, &reverse_oper, NULL);
}