#pragma once
#include <cassert>
#include <cstring>
#include <span>
#include <typeindex>
#include <vector>

#include "AssetSystemStructs.h"

class AssetArray
{
public:
	template<Asset T>
	static AssetArray create();

	void reserve(size_t newCapacity);

	template <Asset T>
	T& push(const T& newElement);

	template <Asset T, typename... Args>
	T& emplace(Args&&... args);

	template <Asset T>
	T& at(size_t index);

	template <Asset T>
	const T& at(size_t index) const;

	[[nodiscard]] size_t size() const;

	template <Asset T>
	[[nodiscard]] bool isExactType() const;

	template <Asset T>
	UUID destruct(size_t index);

	template<Asset T>
	std::span<T> getSpan();

private:
	std::vector<std::byte> assetData;
	size_t assetSize;
	std::type_index assetType;

	AssetArray(const std::type_index& assetType, const size_t& assetSize);
};

template <Asset T>
AssetArray AssetArray::create()
{
	return {std::type_index(typeid(T)), sizeof(T)};
}

template <Asset T>
T& AssetArray::push(const T& newElement)
{
	assert(isExactType<T>());
	auto* data = static_cast<std::byte*>(&newElement);
	std::span<std::byte> dataSpan{data, assetSize};
	assetData.insert(assetData.end(), dataSpan.begin(), dataSpan.end());
	return this->at<T>(size() - 1);
}

template <Asset T, typename... Args>
T& AssetArray::emplace(Args&&... args)
{
	assert(isExactType<T>());
	const auto index{assetData.size()};
	assetData.resize(assetData.size() + assetSize); // TODO: Ideally this would not init the bytes
	auto* data = &assetData[index];
	T* object{new (data) T(std::forward<Args>(args)...)};
	return *object;
}

template <Asset T>
T& AssetArray::at(const size_t index)
{
	assert(isExactType<T>());
	std::byte* object{&assetData[index * assetSize]};
	return *reinterpret_cast<T*>(object);
}

template <Asset T>
const T& AssetArray::at(const size_t index) const
{
	assert(isExactType<T>());
	const std::byte* object{&assetData[index * assetSize]};
	return *reinterpret_cast<const T*>(object);
}

template <Asset T>
bool AssetArray::isExactType() const
{
	return std::type_index{typeid(T)} == assetType;
}

template <Asset T>
UUID AssetArray::destruct(const size_t index)
{
	assert(isExactType<T>());
	T& object = at<T>(index);
	object.~T();
	const size_t lastIndex{(size() - 1) * assetSize};
	std::memcpy(assetData.data() + index * assetSize, assetData.data() + lastIndex, assetSize);
	assetData.erase(assetData.begin() + static_cast<uint32_t>(lastIndex), assetData.end());
	return at<T>(index).getUUID();
}

template<Asset T>
std::span<T> AssetArray::getSpan()
{
	assert(isExactType<T>());
	return {reinterpret_cast<T*>(assetData.data()), size()};
}
