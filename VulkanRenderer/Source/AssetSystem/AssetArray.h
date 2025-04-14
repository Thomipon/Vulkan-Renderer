#pragma once
#include <cassert>
#include <span>
#include <typeindex>
#include <vector>

class AssetArray
{
public:
    void reserve(size_t newCapacity);

    template<typename T>
    T& push(const T& newElement);

    template<typename T>
    T& at(size_t index);

    template<typename T>
    const T& at(size_t index) const;

    [[nodiscard]] size_t size() const;

    template<typename T>
    [[nodiscard]] bool isExactType() const;
private:
    std::vector<std::byte> assetData;
    size_t assetSize;
    std::type_index assetType;

    AssetArray(const std::type_index& assetType, const size_t& assetSize);
};

template<typename T>
T & AssetArray::push(const T &newElement)
{
    assert(isExactType<T>());
    auto* data = static_cast<std::byte*>(&newElement);
    std::span<std::byte> dataSpan{data, assetSize};
    assetData.insert(assetData.end(), dataSpan.begin(), dataSpan.end());
    return this->at<T>(size() - 1);
}

template<typename T>
T & AssetArray::at(const size_t index) {
    assert(isExactType<T>());
    std::byte* object{&assetData[index * assetSize]};
    return *static_cast<T*>(object);
}

template<typename T>
const T & AssetArray::at(size_t index) const {
    assert(isExactType<T>());
    const std::byte* object{&assetData[index * assetSize]};
    return *static_cast<const T*>(object);
}

template<typename T>
bool AssetArray::isExactType() const {
    return std::type_index{typeid(T)} == assetType;
}
