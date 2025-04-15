//
// Created by Thomas on 15/04/2025.
//

#include "AssetBase.hpp"

AssetBase::~AssetBase() = default;

UUID AssetBase::getUUID() const
{
	return uuid;
}

void AssetBase::setUUID(const UUID& uuid)
{
	this->uuid = uuid;
}
