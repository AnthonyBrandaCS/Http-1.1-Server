#ifndef FILE_MAP_H
#define FILE_MAP_H

#include <iostream>
#include <fstream>
#include <map>

#include <unistd.h>
#include <string.h>

class FileMap
{
    private:
        std::map<std::string, std::string> m_fileMap;
        
        std::string readFile(std::string path);
    
    public:
        FileMap(std::string rootDir);
        std::string getFile(std::string filePath);
};

#endif