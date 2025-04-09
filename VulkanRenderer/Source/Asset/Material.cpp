//
// Created by Thomas on 30/03/2025.
//

#include "Material.hpp"

#include <array>


#include "ShaderCompiler.hpp"

void Material::compile(const SlangCompiler& compiler)
{
	auto rasterModule{compiler.loadModule("Core/mainRaster")};
	auto vertEntry{SlangCompiler::findEntryPoint(rasterModule, "vertexMain")};
	auto fragEntry{SlangCompiler::findEntryPoint(rasterModule, "fragmentMain")};

	auto materialModule{compiler.loadModule("BRDF/phong")};
	auto material{materialModule->getLayout()->findTypeByName("ConstantPhongMaterial")};

	auto lightModule{compiler.loadModule("Core/lights")};

	auto composedProgram{compiler.composeProgram({rasterModule, vertEntry, fragEntry, materialModule, lightModule})};

	auto light{lightModule->getLayout()->findTypeByName("DirectionalLight")};

	std::array specializationArgs
	{
		slang::SpecializationArg{
			slang::SpecializationArg::Kind::Type,
			light
		},
		slang::SpecializationArg{
			slang::SpecializationArg::Kind::Type,
			material
		},
	};
	ComPtr<slang::IComponentType> specializedCode{SlangCompiler::specializeProgram(composedProgram, specializationArgs)};
	/*std::vector<slang::SpecializationArg> specializationArgs =
	{
		{
			slang::SpecializationArg::Kind::Type,
			materialModule->getLayout()->findTypeByName("ConstantPhongMaterial")
		}
	};*/

	//auto specializedFrag = SlangCompiler::specializeEntryPoint(fragEntry, specializationArgs);

	vertexSpirv = SlangCompiler::getSprirV(SlangCompiler::linkProgram(specializedCode));
	//vertexSpirv = SlangCompiler::getSprirV(SlangCompiler::linkProgram(compiler.composeProgram({rasterModule, vertEntry, materialModule})));
	//fragmentSpirv = SlangCompiler::getSprirV(SlangCompiler::linkProgram(compiler.composeProgram({rasterModule, fragEntry, materialModule})));
}
