#pragma once
#include "AssetSystem/AssetSystemStructs.h"

class AssetBase
{
public:
	virtual ~AssetBase() = 0;

	[[nodiscard]] UUID getUUID() const;
private:
	void setUUID(const UUID& uuid);

	UUID uuid;

	friend class AssetManager;
};
