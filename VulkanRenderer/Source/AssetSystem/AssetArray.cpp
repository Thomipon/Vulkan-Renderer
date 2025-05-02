//
// Created by Thomas on 14.04.2025.
//

#include "AssetArray.hpp"

size_t AssetArray::size() const
{
	return count;
}

AssetArray::AssetArray(const std::type_index& assetType, const size_t& assetSize)
	: assetData(nullptr), assetSize(assetSize), assetType(assetType), count(0), capacity(0)
{
}
