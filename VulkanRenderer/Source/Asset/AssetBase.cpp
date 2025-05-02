//
// Created by Thomas on 15/04/2025.
//

#include "AssetBase.hpp"

AssetBase::AssetBase(const std::string& name)
	: name(name)
{
}

AssetBase::AssetBase(std::string&& name)
	: name(std::move(name))
{
}

AssetBase::~AssetBase() = default;

UUID AssetBase::getUUID() const
{
	return uuid;
}

std::string_view AssetBase::getName() const
{
	return name;
}

void AssetBase::setUUID(const UUID& uuid)
{
	this->uuid = uuid;
}
