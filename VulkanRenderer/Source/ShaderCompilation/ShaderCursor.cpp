//
// Created by Thomas on 02/04/2025.
//

#include "ShaderCursor.hpp"

void ShaderCursor::write(const void* data, size_t size)
{
	shaderObject->write(offset, data, size);
}

ShaderCursor ShaderCursor::field(const char* name) const
{
	return field(typeLayout->findFieldIndexByName(name));
}

ShaderCursor ShaderCursor::field(uint32_t index) const
{
	slang::VariableLayoutReflection* field{typeLayout->getFieldByIndex(index)};
	ShaderCursor result = *this;
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
