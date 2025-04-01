#pragma once
#include <slang/slang-com-ptr.h>

class SlangCompiler;

class Material {

public:
	void compile(const SlangCompiler& compiler);

	Slang::ComPtr<slang::IBlob> vertexSpirv;
	Slang::ComPtr<slang::IBlob> fragmentSpirv;
};

