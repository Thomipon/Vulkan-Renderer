//
// Created by Thomas on 14.04.2025.
//

#include "AssetManager.h"

void AssetManager::increaseRefCount(const size_t assetUUID)
{
    if (assetUUID != 0) {
        assetInfosByUUID.find(assetUUID)->second.refCount += 1;
    }
}

void AssetManager::decreaseRefCount(const size_t assetUUID) {
    if (assetUUID != 0) {
        auto& assetInfo{assetInfosByUUID.find(assetUUID)->second};
        assetInfo.refCount -= 1;
        if (assetInfo.refCount == 0) {
            assets[assetInfo.type].destruct(assetInfo.index);
            // TODO: update index of swapped item
        }
    }
}
