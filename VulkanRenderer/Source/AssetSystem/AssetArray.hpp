#pragma once
#include <cassert>
#include <span>
#include <typeindex>

#include "AssetSystemStructs.h"

class AssetArray
{
public:
	template <Asset T>
	static AssetArray create();

	// TODO: Destructor

	template <Asset T>
	void reserve(size_t newCapacity, bool allowOvergrowth = true);

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

	template <Asset T>
	std::span<T> getSpan() const;

private:
	std::byte* assetData;
	size_t assetSize;
	std::type_index assetType;

	size_t count;
	size_t capacity;

	AssetArray(const std::type_index& assetType, const size_t& assetSize);

	template <Asset T>
	void shrinkIfNecessary();

	template <Asset T>
	void moveObjects(std::byte* src, std::byte* dst);

	template <Asset T>
	void reallocate();

	static constexpr size_t growthRate{2};
};

template <Asset T>
AssetArray AssetArray::create()
{
	return {std::type_index(typeid(T)), sizeof(T)};
}

template <Asset T>
void AssetArray::reserve(const size_t newCapacity, const bool allowOvergrowth)
{
	if (newCapacity > capacity)
	{
		capacity = allowOvergrowth ? (capacity + 1) * growthRate : newCapacity;
		reallocate<T>();
	}
}

template <Asset T>
T& AssetArray::push(const T& newElement)
{
	return emplace<T>(newElement);
}

template <Asset T, typename... Args>
T& AssetArray::emplace(Args&&... args)
{
	assert(isExactType<T>());
	const auto index{size()};
	reserve<T>(index + 1);
	const auto offset{index * assetSize};
	auto* data = &assetData[offset];
	T* object{new(data) T(std::forward<Args>(args)...)};
	++count;
	return *object;
}

template <Asset T>
T& AssetArray::at(const size_t index)
{
	assert(isExactType<T>());
	assert(index < size());
	std::byte* object{&assetData[index * assetSize]};
	return *reinterpret_cast<T*>(object);
}

template <Asset T>
const T& AssetArray::at(const size_t index) const
{
	assert(isExactType<T>());
	assert(index < size());
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
	T* object = &at<T>(index);
	object->~T();
	const size_t lastIndex{(size() - 1)};

	UUID movedElement{};
	if (lastIndex > index)
	{
		T* lastObject{new(object) T(std::move(at<T>(lastIndex)))};
		movedElement = lastObject->getUUID();
	}
	count = lastIndex;
	shrinkIfNecessary<T>();
	return movedElement;
}

template <Asset T>
std::span<T> AssetArray::getSpan() const
{
	assert(isExactType<T>());
	return {reinterpret_cast<T*>(assetData), size()};
}

template <Asset T>
void AssetArray::shrinkIfNecessary()
{
	if (count < capacity / (growthRate * growthRate))
	{
		capacity = capacity / growthRate;
		reallocate<T>();
	}
}

template <Asset T>
void AssetArray::moveObjects(std::byte* src, std::byte* dst)
{
	for (size_t i = 0; i < size(); ++i)
	{
		const auto offset{i * assetSize};
		T* object{reinterpret_cast<T*>(src + offset)};
		new (dst + offset) T(std::move(*object));
	}
}

template <Asset T>
void AssetArray::reallocate()
{
	if (capacity == 0)
	{
		std::free(assetData);
		assetData = nullptr;
		return;
	}
	std::byte* newData{static_cast<std::byte*>(std::malloc(assetSize * capacity))};
	moveObjects<T>(assetData, newData);
	std::free(assetData);
	assetData = newData;
}
