//
// Created by Thomas on 14.04.2025.
//

#include "AssetArray.h"

void AssetArray::reserve(const size_t newCapacity)
{
    assetData.reserve(newCapacity * assetSize);
}

size_t AssetArray::size() const {
    return assetData.size() / assetSize;
}

AssetArray::AssetArray(const std::type_index &assetType, const size_t &assetSize)
    : assetSize(assetSize), assetType(assetType)
{

}
