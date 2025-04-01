//
// Created by Thomas on 30/03/2025.
//

#include "Material.hpp"


#include "ShaderCompiler.hpp"

void Material::compile(const SlangCompiler& compiler)
{
	auto rasterModule = compiler.loadModule("Core/mainRaster");
	auto vertEntry = SlangCompiler::findEntryPoint(rasterModule, "vertexMain");
	auto fragEntry = SlangCompiler::findEntryPoint(rasterModule, "fragmentMain");

	auto materialModule = compiler.loadModule("BRDF/phong");
	auto material = materialModule->getLayout()->findTypeByName("ConstantPhongMaterial");
	std::vector<slang::SpecializationArg> specializationArgs =
	{
		{
			slang::SpecializationArg::Kind::Type,
			materialModule->getLayout()->findTypeByName("ConstantPhongMaterial")
		}
	};

	//auto specializedFrag = SlangCompiler::specializeEntryPoint(fragEntry, specializationArgs);

	vertexSpirv = SlangCompiler::getSprirV(SlangCompiler::linkProgram(compiler.composeProgram({rasterModule, vertEntry})));
	fragmentSpirv = SlangCompiler::getSprirV(SlangCompiler::linkProgram(compiler.composeProgram({rasterModule, fragEntry, })));
}
