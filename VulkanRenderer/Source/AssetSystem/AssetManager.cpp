//
// Created by Thomas on 14.04.2025.
//

#include "AssetManager.hpp"

void AssetManager::increaseRefCount(const size_t assetUUID)
{
	if (assetUUID != 0)
	{
		assetInfosByUUID.find(assetUUID)->second.refCount += 1;
	}
}
