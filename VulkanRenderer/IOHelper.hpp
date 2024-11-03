#pragma once

#include <vector>
#include <string>
#include <fstream>

inline std::vector<char> readFile(const std::string& filePath)
{
    std::ifstream file{filePath, std::ios::ate | std::ios::binary};

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file " + filePath);
    }

    size_t fileSize(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    return buffer;
}
