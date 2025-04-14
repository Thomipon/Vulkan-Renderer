#pragma once
#include <map>
#include <typeindex>

#include "AssetArray.h"
#include "AssetSystemStructs.h"

class AssetManager
{

private:
    std::map<std::type_index, AssetArray> assets;
    std::map<size_t, AssetInfo> assetInfosByUUID;

    void increaseRefCount(size_t assetUUID);
    void decreaseRefCount(size_t assetUUID);

    template<typename T>
    friend struct AssetHandle;
};

