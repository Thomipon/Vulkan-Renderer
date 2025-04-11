//
// Created by Thomas on 02/04/2025.
//

#include "ShaderCursor.hpp"

#include <iostream>

#include "Debug/SlangDebug.hpp"

ShaderCursor::ShaderCursor(ShaderObject* shaderObject)
	: shaderObject(shaderObject), typeLayout(shaderObject->typeLayout)
{
}

void ShaderCursor::write(const void* data, size_t size)
{
	shaderObject->write(offset, data, size);
}

void ShaderCursor::writeTexture(const TextureImage& texture)
{
	shaderObject->writeTexture(offset, texture);
}

void ShaderCursor::writeSampler(const TextureImage& texture)
{
	shaderObject->writeSampler(offset, texture);
}

ShaderCursor ShaderCursor::field(const char* name) const
{
	return field(typeLayout->findFieldIndexByName(name));
}

ShaderCursor ShaderCursor::field(uint32_t index) const
{
	slang::VariableLayoutReflection* field{typeLayout->getFieldByIndex(index)};
	ShaderCursor result{*this};
	result.typeLayout = field->getTypeLayout();
	result.offset.byteOffset += field->getOffset();
	result.offset.bindingIndex += typeLayout->getFieldBindingRangeOffset(index);

	return result;
}

ShaderCursor ShaderCursor::field(const std::string_view& name) const
{
	return field(name.data());
}

ShaderCursor ShaderCursor::element(uint32_t index) const
{
	slang::TypeLayoutReflection* element{typeLayout->getElementTypeLayout()};

	ShaderCursor result = *this;
	result.typeLayout = element;
	result.offset.byteOffset += index * element->getStride();

	result.offset.bindingArrayElement *= typeLayout->getElementCount();
	result.offset.bindingArrayElement += index;

	return result;
}

const ShaderOffset& ShaderCursor::getOffset() const
{
	return offset;
}

void ShaderCursor::printLayout() const
{
	SlangDebug::SlangPrinter printer{};
	printer << typeLayout;
	std::cout << printer << std::endl;
}
