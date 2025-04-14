#pragma once
#include "AssetManager.h"
#include "AssetSystemStructs.h"

template<typename T>
struct AssetHandle {

public:

    T& operator*() const;
    T* operator->() const;

    AssetHandle(const AssetHandle& other);
    AssetHandle(AssetHandle&& other) noexcept;
    AssetHandle& operator=(const AssetHandle& other);
    AssetHandle& operator=(AssetHandle&& other) noexcept;
    ~AssetHandle();

private:
    AssetManager& assetManager;

    UUID uuid;
};

template<typename T>
T & AssetHandle<T>::operator*() const {
    const AssetInfo assetInfo{assetManager.assetInfosByUUID[uuid.value]};
    return assetManager.assets[assetInfo.type].at<T>(assetInfo.index);
}

template<typename T>
T * AssetHandle<T>::operator->() const {
    return **this;
}

template<typename T>
AssetHandle<T>::AssetHandle(const AssetHandle &other)
    : assetManager(other.assetManager), uuid(other.uuid)
{
    assetManager.increaseRefCount(uuid.value);
}

template<typename T>
AssetHandle<T>::AssetHandle(AssetHandle &&other) noexcept
    : assetManager(other.assetManager), uuid(other.uuid)
{
    other.uuid = {};
}

template<typename T>
AssetHandle<T> & AssetHandle<T>::operator=(const AssetHandle &other)
{
    assert(&assetManager == &other.assetManager);
    uuid = other.uuid;
    assetManager.increaseRefCount(uuid.value);
    return *this;
}

template<typename T>
AssetHandle<T> & AssetHandle<T>::operator=(AssetHandle &&other) noexcept {
    assert(&assetManager == &other.assetManager);
    uuid = other.uuid;
    other.uuid = {};
    return *this;
}

template<typename T>
AssetHandle<T>::~AssetHandle()
{
    if (uuid.value != 0) {
        assetManager.decreaseRefCount(uuid.value);
    }
}

