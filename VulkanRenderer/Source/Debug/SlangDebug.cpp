//
// Created by Thomas on 10.04.2025.
//

#include "SlangDebug.hpp"

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(slang::ProgramLayout* program)
{
	*this << "global scope:\n" << BeginIndent{} << PrintIndent{};
	printScope(program->getGlobalParamsVarLayout());
	*this << EndIndent{} << "entry points:\n" << BeginIndent{} << PrintIndent{};
	for (int i = 0; i < program->getEntryPointCount(); ++i)
	{
		*this << '\n' << PrintIndent{} << program->getEntryPointByIndex(i);
	}
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(slang::VariableReflection* var)
{
	return *this << "name: \"" << var->getName() << "\"\n" << PrintIndent{} << "type:\n" << BeginIndent{} <<
		PrintIndent{} << var->getType() << EndIndent{};
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(slang::TypeReflection* type)
{
	*this << "name: \"" << type->getName() << "\"\n" << PrintIndent{} << "kind: " << type->getKind();

	switch (type->getKind())
	{
	default:
	case slang::TypeReflection::Kind::None:
		break;
	case slang::TypeReflection::Kind::Struct:
		printStructType(type);
		break;
	case slang::TypeReflection::Kind::Array:
		printArrayType(type);
		break;
	case slang::TypeReflection::Kind::Matrix:
		printMatrixType(type);
		break;
	case slang::TypeReflection::Kind::Vector:
		printVectorType(type);
		break;
	case slang::TypeReflection::Kind::Scalar:
		printScalarType(type);
		break;
	case slang::TypeReflection::Kind::ConstantBuffer:
	case slang::TypeReflection::Kind::TextureBuffer:
	case slang::TypeReflection::Kind::ParameterBlock:
	case slang::TypeReflection::Kind::ShaderStorageBuffer:
		printSingleElementContainerType(type);
		break;
	case slang::TypeReflection::Kind::Resource:
		printResourceType(type);
		break;
	case slang::TypeReflection::Kind::SamplerState:
		break;
	case slang::TypeReflection::Kind::GenericTypeParameter:
		break;
	case slang::TypeReflection::Kind::Interface:
		break;
	case slang::TypeReflection::Kind::OutputStream:
		break;
	case slang::TypeReflection::Kind::Specialized:
		break;
	case slang::TypeReflection::Kind::Feedback:
		break;
	case slang::TypeReflection::Kind::Pointer:
		break;
	case slang::TypeReflection::Kind::DynamicResource:
		break;
	}
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(slang::TypeReflection::Kind kind)
{
	switch (kind)
	{
	case slang::TypeReflection::Kind::None:
		return *this << "none";
	case slang::TypeReflection::Kind::Struct:
		return *this << "struct";
	case slang::TypeReflection::Kind::Array:
		return *this << "array";
	case slang::TypeReflection::Kind::Matrix:
		return *this << "matrix";
	case slang::TypeReflection::Kind::Vector:
		return *this << "vector";
	case slang::TypeReflection::Kind::Scalar:
		return *this << "scalar";
	case slang::TypeReflection::Kind::ConstantBuffer:
		return *this << "constant buffer";
	case slang::TypeReflection::Kind::Resource:
		return *this << "resource";
	case slang::TypeReflection::Kind::SamplerState:
		return *this << "sampler state";
	case slang::TypeReflection::Kind::TextureBuffer:
		return *this << "texture buffer";
	case slang::TypeReflection::Kind::ShaderStorageBuffer:
		return *this << "shader storage buffer";
	case slang::TypeReflection::Kind::ParameterBlock:
		return *this << "parameter block";
	case slang::TypeReflection::Kind::GenericTypeParameter:
		return *this << "generic type parameter";
	case slang::TypeReflection::Kind::Interface:
		return *this << "interface";
	case slang::TypeReflection::Kind::OutputStream:
		return *this << "output stream";
	case slang::TypeReflection::Kind::Specialized:
		return *this << "specialized";
	case slang::TypeReflection::Kind::Feedback:
		return *this << "feedback";
	case slang::TypeReflection::Kind::Pointer:
		return *this << "pointer";
	case slang::TypeReflection::Kind::DynamicResource:
		return *this << "dynamic resource";
	}
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(slang::TypeReflection::ScalarType type)
{
	switch (type)
	{
	case slang::TypeReflection::None:
		return *this << "none";
	case slang::TypeReflection::Void:
		return *this << "void";
	case slang::TypeReflection::Bool:
		return *this << "bool";
	case slang::TypeReflection::Int32:
		return *this << "int32";
	case slang::TypeReflection::UInt32:
		return *this << "uint32";
	case slang::TypeReflection::Int64:
		return *this << "int64";
	case slang::TypeReflection::UInt64:
		return *this << "uint64";
	case slang::TypeReflection::Float16:
		return *this << "float16";
	case slang::TypeReflection::Float32:
		return *this << "float32";
	case slang::TypeReflection::Float64:
		return *this << "float64";
	case slang::TypeReflection::Int8:
		return *this << "int8";
	case slang::TypeReflection::UInt8:
		return *this << "uint8";
	case slang::TypeReflection::Int16:
		return *this << "int16";
	case slang::TypeReflection::UInt16:
		return *this << "uint16";
	}
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(SlangResourceShape shape)
{
	switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
	{
	case SLANG_RESOURCE_NONE:
		return *this << "none";
	case SLANG_TEXTURE_1D:
		*this << "texture1D";
	case SLANG_TEXTURE_2D:
		*this << "texture2D";
	case SLANG_TEXTURE_3D:
		*this << "texture3D";
	case SLANG_TEXTURE_CUBE:
		*this << "texture cube";
	case SLANG_TEXTURE_BUFFER:
		*this << "texture buffer";
	case SLANG_STRUCTURED_BUFFER:
		*this << "structured buffer";
	case SLANG_BYTE_ADDRESS_BUFFER:
		*this << "byte address buffer";
	case SLANG_RESOURCE_UNKNOWN:
		*this << "unknown";
	case SLANG_ACCELERATION_STRUCTURE:
		*this << "acceleration structure";
	case SLANG_TEXTURE_SUBPASS:
		*this << "texture subpass";
	default:
		break;
	}

	if (shape & SLANG_TEXTURE_FEEDBACK_FLAG)
	{
		*this << " feedback";
	}
	if (shape & SLANG_TEXTURE_SHADOW_FLAG)
	{
		*this << " shadow";
	}
	if (shape & SLANG_TEXTURE_ARRAY_FLAG)
	{
		*this << " array";
	}
	if (shape & SLANG_TEXTURE_MULTISAMPLE_FLAG)
	{
		*this << " multisample";
	}

	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(SlangResourceAccess access)
{
	switch (access)
	{
	case SLANG_RESOURCE_ACCESS_NONE:
		return *this << "none";
	case SLANG_RESOURCE_ACCESS_READ:
		return *this << "read";
	case SLANG_RESOURCE_ACCESS_READ_WRITE:
		return *this << "read-write";
	case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
		return *this << "raster-ordered";
	case SLANG_RESOURCE_ACCESS_APPEND:
		return *this << "append";
	case SLANG_RESOURCE_ACCESS_CONSUME:
		return *this << "consume";
	case SLANG_RESOURCE_ACCESS_WRITE:
		return *this << "write";
	case SLANG_RESOURCE_ACCESS_FEEDBACK:
		return *this << "feedback";
	case SLANG_RESOURCE_ACCESS_UNKNOWN:
		return *this << "unknown";
	}
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(slang::ParameterCategory category)
{
	switch (category)
	{
	case slang::None:
		return *this << "none";
	case slang::Mixed:
		return *this << "mixed";
	case slang::ConstantBuffer:
		return *this << "constant buffer";
	case slang::ShaderResource:
		return *this << "shader resource";
	case slang::UnorderedAccess:
		return *this << "unordered access";
	case slang::VaryingInput:
		return *this << "varying input";
	case slang::VaryingOutput:
		return *this << "varying output";
	case slang::SamplerState:
		return *this << "sampler state";
	case slang::Uniform:
		return *this << "uniform";
	case slang::DescriptorTableSlot:
		return *this << "descriptor table slot";
	case slang::SpecializationConstant:
		return *this << "specialization constant";
	case slang::PushConstantBuffer:
		return *this << "push constant buffer";
	case slang::RegisterSpace:
		return *this << "register space";
	case slang::GenericResource:
		return *this << "generic resource";
	case slang::RayPayload:
		return *this << "ray payload";
	case slang::HitAttributes:
		return *this << "hit attributes";
	case slang::CallablePayload:
		return *this << "callable payload";
	case slang::ShaderRecord:
		return *this << "shader record";
	case slang::ExistentialTypeParam:
		return *this << "existential type param";
	case slang::ExistentialObjectParam:
		return *this << "existential object param";
	case slang::SubElementRegisterSpace:
		return *this << "subelement register space";
	case slang::InputAttachmentIndex:
		return *this << "input attachment index";
	case slang::MetalArgumentBufferElement:
		return *this << "metal argument buffer element";
	case slang::MetalAttribute:
		return *this << "metal attribute";
	case slang::MetalPayload:
		return *this << "metal payload";
	}
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(SlangStage stage)
{
	switch (stage)
	{
	case SLANG_STAGE_NONE:
		return *this << "none";
	case SLANG_STAGE_VERTEX:
		return *this << "vertex";
	case SLANG_STAGE_HULL:
		return *this << "hull";
	case SLANG_STAGE_DOMAIN:
		return *this << "domain";
	case SLANG_STAGE_GEOMETRY:
		return *this << "geometry";
	case SLANG_STAGE_FRAGMENT:
		return *this << "fragment";
	case SLANG_STAGE_COMPUTE:
		return *this << "compute";
	case SLANG_STAGE_RAY_GENERATION:
		return *this << "ray generation";
	case SLANG_STAGE_INTERSECTION:
		return *this << "ray intersection";
	case SLANG_STAGE_ANY_HIT:
		return *this << "ray any hit";
	case SLANG_STAGE_CLOSEST_HIT:
		return *this << "ray closest hit";
	case SLANG_STAGE_MISS:
		return *this << "ray miss";
	case SLANG_STAGE_CALLABLE:
		return *this << "callable";
	case SLANG_STAGE_MESH:
		return *this << "mesh";
	case SLANG_STAGE_AMPLIFICATION:
		return *this << "amplification";
	}
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(slang::EntryPointReflection* entryPoint)
{
	*this << "name: \"" << entryPoint->getName() << "\"\n"
		<< PrintIndent{} << "stage: " << entryPoint->getStage() << '\n' << PrintIndent{};
	printScope(entryPoint->getVarLayout());
	auto resultVarLayout = entryPoint->getResultVarLayout();
	if (resultVarLayout->getTypeLayout()->getKind() != slang::TypeReflection::Kind::None)
	{
		*this << '\n' << PrintIndent{} << "result:\n" << BeginIndent{} << PrintIndent{} << resultVarLayout << EndIndent{};
	}

	switch (entryPoint->getStage())
	{
	case SLANG_STAGE_COMPUTE:
		{
			SlangUInt sizes[3];
			entryPoint->getComputeThreadGroupSize(3, sizes);

			*this << '\n' << PrintIndent{} << "thread group size:" << BeginIndent{};
			*this << '\n' << PrintIndent{} << "x: " << sizes[0];
			*this << '\n' << PrintIndent{} << "y: " << sizes[1];
			*this << '\n' << PrintIndent{} << "z: " << sizes[2] << EndIndent{};
		}
		break;
	default:
		break;
	}

	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(slang::VariableLayoutReflection* layout)
{
	*this << "name: \"" << layout->getName() << "\"\n"
		<< PrintIndent{} << "relative offset:" << BeginIndent{};
	printOffset(layout);
	*this << EndIndent{} << '\n'
		<< PrintIndent{} << "type layout:\n" <<
		BeginIndent{} << PrintIndent{} << layout->getTypeLayout() << EndIndent{};

	if (layout->getStage() != SlangStage::SLANG_STAGE_NONE)
	{
		*this << '\n' << PrintIndent{} << "semantic:\n" << BeginIndent{} << PrintIndent{}
		<< "name: \"" << layout->getSemanticName() << "\"\n"
		<< PrintIndent{} << "index: " << layout->getSemanticIndex() << EndIndent{};
	}

	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(slang::TypeLayoutReflection* layout)
{
	*this << "name: \"" << layout->getName() << "\"\n" << PrintIndent{} << "kind: " << layout->getKind() << '\n'
		<< PrintIndent{} << "sizes:" << BeginIndent{};
	printSizes(layout);
	*this << EndIndent{};

	if (layout->getSize() > 0)
	{
		*this << '\n' << PrintIndent{} << "alignment (bytes): " << layout->getAlignment() << '\n'
			<< PrintIndent{} << "stride (bytes): " << layout->getStride();
	}

	switch (layout->getKind())
	{
	case slang::TypeReflection::Kind::Struct:
		{
			*this << '\n' << PrintIndent{} << "fields:" << BeginIndent{};
			for (int i = 0; i < layout->getFieldCount(); ++i)
			{
				*this << '\n' << PrintIndent{} << layout->getFieldByIndex(i);
			}
			*this << EndIndent{};
		}
		break;
	case slang::TypeReflection::Kind::Array:
		{
			*this << '\n' << PrintIndent{} << "element count: ";
			const size_t count{layout->getElementCount()};
			if (count == ~size_t{0})
			{
				*this << "unbounded";
			}
			else
			{
				*this << count;
			}
			*this << '\n' << PrintIndent{} << "element type layout: " << layout->getElementTypeLayout();
		}
		break;
	case slang::TypeReflection::Kind::Matrix:
		{
			*this << '\n' << PrintIndent{} << "matrix layout mode: ";
			switch (layout->getMatrixLayoutMode())
			{
			case SLANG_MATRIX_LAYOUT_MODE_UNKNOWN:
				*this << "unknown";
				break;
			case SLANG_MATRIX_LAYOUT_ROW_MAJOR:
				*this << "row major";
				break;
			case SLANG_MATRIX_LAYOUT_COLUMN_MAJOR:
				*this << "column major";
				break;
			}
		}
		break;
	case slang::TypeReflection::Kind::ConstantBuffer:
	case slang::TypeReflection::Kind::ParameterBlock:
	case slang::TypeReflection::Kind::TextureBuffer:
	case slang::TypeReflection::Kind::ShaderStorageBuffer:
		{
			*this << '\n' << PrintIndent{} << "container:" << BeginIndent{};
			printOffset(layout->getContainerVarLayout());
			*this << EndIndent{} << '\n' << PrintIndent{} << "element:" << BeginIndent{};
			printOffset(layout->getElementVarLayout());
			*this << EndIndent{} << '\n' << PrintIndent{} << "type layout:\n"
				<< BeginIndent{} << PrintIndent{} << layout->getElementVarLayout()->getTypeLayout() << EndIndent{};
		}
		break;
	default:
		break;
	}

	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(BeginIndent)
{
	++indent;
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(EndIndent)
{
	--indent;
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(PrintIndent)
{
	for (int i = 0; i < indent; ++i)
	{
		s << "\t";
	}
	return *this;
}

SlangDebug::SlangPrinter& SlangDebug::SlangPrinter::operator<<(const char* str)
{
	if (str)
	{
		s << str;
	}
	else
	{
		s << "none";
	}
	return *this;
}

void SlangDebug::SlangPrinter::printScalarType(slang::TypeReflection* type)
{
	*this << '\n' << BeginIndent{} << PrintIndent{} << "scalar type: " << type->getScalarType() << EndIndent{};
}

void SlangDebug::SlangPrinter::printStructType(slang::TypeReflection* type)
{
	*this << '\n' << BeginIndent{} << PrintIndent{} << "fields:" << BeginIndent{};
	for (int i = 0; i < type->getFieldCount(); ++i)
	{
		*this << '\n' << PrintIndent{} << type->getFieldByIndex(i);
	}
	*this << EndIndent{} << EndIndent{};
}

void SlangDebug::SlangPrinter::printArrayType(slang::TypeReflection* type)
{
	*this << '\n' << BeginIndent{} << PrintIndent{} << "element count: ";
	const size_t count{type->getElementCount()};
	if (count == ~size_t{0})
	{
		*this << "unbounded";
	}
	else
	{
		*this << count;
	}
	*this << '\n' << PrintIndent{} << "element type: " << type->getElementType() << EndIndent{};
}

void SlangDebug::SlangPrinter::printVectorType(slang::TypeReflection* type)
{
	*this << '\n' << BeginIndent{} << PrintIndent{} << "element count: " << type->getElementCount() << '\n'
		<< PrintIndent{} << "element type: " << type->getElementType() << EndIndent{};
}

void SlangDebug::SlangPrinter::printMatrixType(slang::TypeReflection* type)
{
	*this << '\n' << BeginIndent{} << PrintIndent{} << "row count: " << type->getRowCount() << '\n'
		<< PrintIndent{} << "column count: " << type->getColumnCount() << '\n'
		<< PrintIndent{} << "element type: " << type->getElementType() << EndIndent{};
}

void SlangDebug::SlangPrinter::printResourceType(slang::TypeReflection* type)
{
	*this << '\n' << BeginIndent{} << PrintIndent{} << "shape: " << type->getResourceShape() << '\n'
		<< PrintIndent{} << "access: " << type->getResourceAccess() << '\n'
		<< PrintIndent{} << "result type: " << type->getResourceResultType() << EndIndent{};
}

void SlangDebug::SlangPrinter::printSingleElementContainerType(slang::TypeReflection* type)
{
	*this << '\n' << BeginIndent{} << PrintIndent{} << "element type: " << type->getElementType() << EndIndent{};
}

void SlangDebug::SlangPrinter::printOffset(slang::VariableLayoutReflection* layout)
{
	const uint32_t layoutUnitCount{layout->getCategoryCount()};
	for (int i = 0; i < layoutUnitCount; ++i)
	{
		const auto unit{layout->getCategoryByIndex(i)};
		*this << '\n' << PrintIndent{} << "value: " << layout->getOffset(static_cast<SlangParameterCategory>(unit)) << '\n'
			<< PrintIndent{} << "unit: " << unit;

		switch (unit)
		{
		case slang::ParameterCategory::ConstantBuffer:
		case slang::ParameterCategory::ShaderResource:
		case slang::ParameterCategory::UnorderedAccess:
		case slang::ParameterCategory::SamplerState:
		case slang::ParameterCategory::DescriptorTableSlot:
			*this << '\n' << PrintIndent{} << "space: " << layout->getBindingSpace(static_cast<SlangParameterCategory>(unit));
		default:
			break;
		}
	}
}

void SlangDebug::SlangPrinter::printSizes(slang::TypeLayoutReflection* layout)
{
	const uint32_t layoutUnitCount{layout->getCategoryCount()};
	for (int i = 0; i < layoutUnitCount; ++i)
	{
		const auto unit{layout->getCategoryByIndex(i)};
		const auto size{layout->getSize(static_cast<SlangParameterCategory>(unit))};
		*this << '\n' << PrintIndent{} << "value: " << size << '\n'
			<< PrintIndent{} << "unit: " << unit;
	}
}

void SlangDebug::SlangPrinter::printScope(slang::VariableLayoutReflection* layout)
{
	auto scopeTypeLayout{layout->getTypeLayout()};
	switch (scopeTypeLayout->getKind())
	{
	case slang::TypeReflection::Kind::Struct:
		{
			*this << "parameters:" << BeginIndent{};
			for (int i = 0; i < scopeTypeLayout->getFieldCount(); ++i)
			{
				*this << '\n' << PrintIndent{} << scopeTypeLayout->getFieldByIndex(i);
			}
			*this << EndIndent{};
		}
		break;
	case slang::TypeReflection::Kind::ConstantBuffer:
		*this << "automatically-introduced constant buffer:" << BeginIndent{};
		printOffset(scopeTypeLayout->getContainerVarLayout());

		*this << '\n' << PrintIndent{};
		printScope(scopeTypeLayout->getElementVarLayout());
		break;
	case slang::TypeReflection::Kind::ParameterBlock:
		*this << "automatically-introduced parameter block:" << BeginIndent{};
		printOffset(scopeTypeLayout->getContainerVarLayout());

		*this << '\n' << PrintIndent{};
		printScope(scopeTypeLayout->getElementVarLayout());
		break;
	default:
		break;
	}
}

std::ostream& SlangDebug::operator<<(std::ostream& os, const SlangPrinter& printer)
{
	return os << printer.s.view();
}
