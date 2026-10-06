#include "ResourceManager.h"
#include <unordered_map>
#include <fstream>

namespace {
    struct FileInfo {
        int size;
        unsigned char* data;
    };
    std::unordered_map < std::string, FileInfo> files;

    void readFile(std::string filename)
    {
        std::ifstream ifs(filename, std::ios::binary);
        if (ifs) {
            FileInfo f;
            ifs.seekg(0, std::ios::_Seekend);
            f.size = (int)ifs.tellg();
            ifs.seekg(0, std::ios::_Seekbeg);
            f.data = new unsigned char[f.size];
            ifs.read((char*)f.data, f.size);
            ifs.close();
            files[filename] = f;
        }
    }
};
void ResourceManager::Init()
{
    files.clear();
}

void ResourceManager::Reset()
{
    for (auto f : files) {
        delete f.second.data;
    }
    files.clear();
}

unsigned char* ResourceManager::Load(std::string filename)
{
    if (files.find(filename) == files.end()) {
        readFile(filename);
    }
    return files[filename].data;
}

int ResourceManager::Size(std::string filename)
{
    if (files.find(filename) == files.end()) {
        readFile(filename);
    }
    return files[filename].size;
}