/*
Student No.: 111550093
Student Name: I-TING, CHU
Email: itingchu1005@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page. 
*/

#define FUSE_USE_VERSION 30
#define _FILE_OFFSET_BITS 64
#define BLOCK_SIZE 512
#define TAR_FILENAME "test.tar"

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <sstream>
#include <fuse.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

// TAR header structure
struct Header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

// TAR entry structure
struct TarEntry {
    Header header;   // TAR header
    string content;  // File content
};

// Node structure for the directory tree
struct Node {
    TarEntry tar_entry;              // TAR entry information
    string name;                     // File or directory name
    struct stat attributes;          // File or directory attributes
    map<string, Node*> children;     // Child nodes (files and directories)
    string link_target;              // If symlink, target path
    bool is_symlink = false;         // Is this a symbolic link
    
    Node(const string& name) : name(name) {}
};

// Directory tree structure
class DirectoryTree {
public:
    Node* root;

    DirectoryTree() {
        root = new Node("/");
        root->attributes.st_mode = S_IFDIR | 0444; // Root directory permissions
    }

    ~DirectoryTree() {
        clearTree(root);
    }

    // Add a node to the tree
    void addNode(const string& path, const struct stat& attributes, bool is_symlink = false, const string& link_target = "", const TarEntry& tar_entry = {}) {
        vector<string> parts = splitPath(path);
        Node* current = root;

        for (const auto& part : parts) {
            // if not exist, create a new node
            if (current->children.find(part) == current->children.end()) {
                current->children[part] = new Node(part);
            }
            current = current->children[part]; 
        }

        current->attributes = attributes;
        current->tar_entry = tar_entry;
        if (is_symlink) {
            current->is_symlink = true;
            current->link_target = link_target;
        }
    }

    // Find a node in the tree
    Node* findNode(const string& path) {
        vector<string> parts = splitPath(path);
        Node* current = root;

        for (const auto& part : parts) {
            if (current->children.find(part) == current->children.end()) {
                return nullptr;
            }
            current = current->children[part];
        }

        return current;
    }

private:
    // Helper: split path into parts
    vector<string> splitPath(const string& path) {
        vector<string> parts;
        stringstream ss(path);
        string part;

        while (getline(ss, part, '/')) {
            if (!part.empty()) parts.push_back(part);
        }

        return parts;
    }

    // Helper: clear tree recursively
    void clearTree(Node* node) {
        for (auto& [_, child] : node->children) {
            clearTree(child);
        }
        delete node;
    }
};

DirectoryTree directoryTree;

// Load TAR file and populate directory tree
void loadTarFile() {
    ifstream file(TAR_FILENAME, ios::binary);
    if (!file.is_open()) {
        cerr << "Failed to open TAR file." << endl;
        return;
    }

    Header header;
    while (file.read(reinterpret_cast<char*>(&header), BLOCK_SIZE)) {
        // Check for end of archive
        if (header.name[0] == '\0') {
            Header next_header;
            file.read(reinterpret_cast<char*>(&next_header), BLOCK_SIZE);
            if (file.gcount() < BLOCK_SIZE || next_header.name[0] == '\0') {
                break;
            } else {
                file.seekg(-BLOCK_SIZE, ios::cur);
            }
        }

        string file_name(header.name, 100);
        file_name = file_name.substr(0, file_name.find('\0'));

        size_t file_size = strtol(header.size, nullptr, 8);

        struct stat attributes = {};
        attributes.st_mode = (header.type[0] == '5' ? S_IFDIR : (header.type[0] == '2' ? S_IFLNK : S_IFREG)) | strtol(header.mode, nullptr, 8);
        attributes.st_uid = strtol(header.uid, nullptr, 8);
        attributes.st_gid = strtol(header.gid, nullptr, 8);
        attributes.st_mtime = strtol(header.mtime, nullptr, 8);
        attributes.st_size = (header.type[0] == '5' ? 0 : file_size);

        TarEntry tar_entry = {header, ""};
        if (header.type[0] != '5' && header.type[0] != '2' && file_size > 0) {
            vector<char> content(file_size);
            file.read(content.data(), file_size);
            tar_entry.content = string(content.begin(), content.end());
        }

        if (header.type[0] == '2') {
            string link_target(header.linkname, 100);
            link_target = link_target.substr(0, link_target.find('\0'));
            directoryTree.addNode(file_name, attributes, true, link_target, tar_entry);
        } else {
            directoryTree.addNode(file_name, attributes, false, "", tar_entry);
        }

        // Skip padding if needed
        long skip_blocks = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        file.seekg(skip_blocks * BLOCK_SIZE - file_size, ios::cur);
    }

    file.close();
}

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    Node* dir = directoryTree.findNode(path);
    if (!dir || !(dir->attributes.st_mode & S_IFDIR)) {
        return -ENOENT;
    }

    filler(buffer, ".", nullptr, 0);
    filler(buffer, "..", nullptr, 0);

    for (const auto& [name, child] : dir->children) {
        filler(buffer, name.c_str(), nullptr, 0);
    }

    return 0;
}

int my_getattr(const char *path, struct stat *st) {
    Node* node = directoryTree.findNode(path);
    if (!node) {
        return -ENOENT;
    }

    *st = node->attributes;
    return 0;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
    Node* node = directoryTree.findNode(path);
    if (!node || (node->attributes.st_mode & S_IFDIR)) {
        return -EISDIR; // Cannot read directories
    }

    size_t file_size = node->attributes.st_size;
    if (offset >= file_size) {
        return 0; // Offset is beyond file size
    }

    size_t bytes_to_read = min(size, static_cast<size_t>(file_size - offset));
    memcpy(buffer, node->tar_entry.content.data() + offset, bytes_to_read);

    return bytes_to_read;
}

int my_readlink(const char *path, char *buffer, size_t size) {
    Node* node = directoryTree.findNode(path);
    if (!node || !node->is_symlink) {
        return -EINVAL; // Not a symlink
    }

    strncpy(buffer, node->link_target.c_str(), size - 1);
    buffer[size - 1] = '\0'; // Null-terminate

    return 0;
}

static struct fuse_operations op;

int main(int argc, char *argv[]) {
    memset(&op, 0, sizeof(op));
    op.getattr = my_getattr;
    op.readdir = my_readdir;
    op.read = my_read;
    op.readlink = my_readlink;
    loadTarFile();
    return fuse_main(argc, argv, &op, nullptr);
}