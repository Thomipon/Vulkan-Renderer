#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

inline std::vector<char> readFile(const std::filesystem::path& filePath)
{
    static const std::filesystem::path baseDir{"../../VulkanRenderer/"}; // TODO: This is awful and requires better file system management
    std::ifstream file{baseDir / filePath, std::ios::ate | std::ios::binary};
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file " + std::filesystem::absolute(baseDir / filePath).string());
    }

    size_t fileSize(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    return buffer;
}
