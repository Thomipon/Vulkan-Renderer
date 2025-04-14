#pragma once
#include <cstdint>

struct UUID {
    size_t value{0};
};

struct AssetInfo
{
    UUID uuid{};
    uint32_t refCount{};
    std::type_index type;
    uint32_t index{};
};
