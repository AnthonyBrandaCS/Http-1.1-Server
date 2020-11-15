#include "FileMap.h"

FileMap::FileMap(std::string rootDir)
{
    FILE *fpipe;
    std::string command = "find " + rootDir + " -type f";

    if((fpipe = (FILE*)popen(command.c_str(), "r")) == 0)
    {
        std::cerr << "Cannot read files, popen failed: " << strerror(errno);
        exit(EXIT_FAILURE);
    }
    
    char c = 0;
    int count = 0;
    std::string result = "";

    while(fread(&c, sizeof c, 1, fpipe))
    {
		result += c;
		
		if(c == '\n')
        {
			count++;
        }
	}

    pclose(fpipe);

    std::string files[count];
    int fileIndex = 0;

    for(int i = 0; i < result.size(); i++)
    {
        if(result[i] == '\n')
        {
            m_fileMap[files[fileIndex]] = readFile(files[fileIndex]);

            fileIndex++;
        }
        else
        {
            files[fileIndex] += result[i];
        }
    }
}

//private
std::string FileMap::readFile(std::string path)
{
	std::ifstream file;
	
	file.open(path);
	
	std::string output = "";
	
	std::fstream::traits_type::int_type x;
	
	while((x = file.get()) != std::fstream::traits_type::eof())
    {
        // TODO: Learn how to make this more efficient.
        // I know the operator+ is frowned appon, but
        // this only happens before the server starts
        // so it doesn't really matter much.
		output += x;
    }
	
	file.close();
	
	return output;
}


// public
std::string FileMap::getFile(std::string path)
{
	return m_fileMap[path];
}