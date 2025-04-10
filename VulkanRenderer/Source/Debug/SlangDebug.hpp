#pragma once
#include <ostream>
#include <sstream>
#include <slang/slang.h>

namespace SlangDebug
{
	struct SlangPrinter
	{
		SlangPrinter& operator<<(slang::VariableReflection* var);
		SlangPrinter& operator<<(slang::TypeReflection* type);
		SlangPrinter& operator<<(slang::TypeReflection::Kind kind);
		SlangPrinter& operator<<(slang::TypeReflection::ScalarType type);
		SlangPrinter& operator<<(SlangResourceShape shape);
		SlangPrinter& operator<<(SlangResourceAccess access);
		SlangPrinter& operator<<(slang::ParameterCategory category);

		SlangPrinter& operator<<(slang::VariableLayoutReflection* layout);

		struct BeginIndent
		{
		};

		struct EndIndent
		{
		};

		struct PrintIndent
		{
		};

		SlangPrinter& operator<<(BeginIndent);
		SlangPrinter& operator<<(EndIndent);
		SlangPrinter& operator<<(PrintIndent);

		template <typename T>
		SlangPrinter& operator<<(const T& t)
		{
			s << t;
			return *this;
		}

	private:
		int indent{0};
		std::stringstream s;

		void printScalarType(slang::TypeReflection* type);
		void printStructType(slang::TypeReflection* type);
		void printArrayType(slang::TypeReflection* type);
		void printVectorType(slang::TypeReflection* type);
		void printMatrixType(slang::TypeReflection* type);
		void printResourceType(slang::TypeReflection* type);
		void printSingleElementContainerType(slang::TypeReflection* type);

		void printOffset(slang::VariableLayoutReflection* layout);
	};
}
