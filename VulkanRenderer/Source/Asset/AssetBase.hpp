#pragma once
#include <string>
#include <string_view>

#include "AssetSystem/AssetSystemStructs.h"

class AssetBase
{
public:
	AssetBase() = default;
	explicit AssetBase(const std::string& name);
	explicit AssetBase(std::string&& name);
	virtual ~AssetBase() = 0;

	[[nodiscard]] UUID getUUID() const;
	[[nodiscard]] std::string_view getName() const;
protected:
	std::string name{""};

private:
	void setUUID(const UUID& uuid);

	UUID uuid;

	friend class AssetManager;
};

template <typename T>
concept Asset = std::is_base_of_v<AssetBase, T>;
