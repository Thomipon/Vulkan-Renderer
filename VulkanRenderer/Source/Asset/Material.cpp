//
// Created by Thomas on 30/03/2025.
//

#include "Material.hpp"

#include <array>

#include "Renderer.hpp"
#include "ShaderCompiler.hpp"
#include "Vertex.hpp"
#include "Debug/SlangDebug.hpp"
#include "Scene/Light/BasicLights.hpp"
#include "Scene/Light/CompositeLights.hpp"
#include "Scene/Light/UniversalLightEnvironment.hpp"

Material::Material(const std::string_view& materialModuleName, const std::string_view& materialTypeName)
	: AssetBase((std::string{materialModuleName} + "/").append(materialTypeName)),
	  pipelineLayout(nullptr), pipeline(nullptr), materialModuleName(materialModuleName), materialTypeName(materialTypeName)
{
}

void Material::compile(const SlangCompiler& compiler, const Renderer& app)
{
	auto [materialModule, materialType]{loadMaterial(materialModuleName, materialTypeName, compiler)};
	auto [newProgram, existentialObjects]{compileMaterialProgram(materialModule, materialType, compiler)};
	program = newProgram;
	spirv = compileSpirv(program);
	shaderLayout = std::make_unique<VulkanShaderObjectLayout>(SlangCompiler::getProgramLayout(program)->getGlobalParamsVarLayout(), existentialObjects, app);
	pipelineLayout = createPipelineLayout(*shaderLayout, app);
	pipeline = createPipeline(spirv, pipelineLayout, app);
}

std::pair<Slang::ComPtr<slang::IModule>, slang::TypeReflection*> Material::loadMaterial(const std::string_view& materialModuleName, const std::string_view& materialType, const SlangCompiler& compiler)
{
	auto materialModule{compiler.loadModule(materialModuleName)};
	auto material{materialModule->getLayout()->findTypeByName(materialType.data())};
	return {materialModule, material};
}

Spirv Material::compileSpirv(const Slang::ComPtr<slang::IComponentType>& program)
{
	auto linked{SlangCompiler::linkProgram(program)};
	return {SlangCompiler::getSprirV(linked, 0), SlangCompiler::getSprirV(linked, 1)};
}

std::pair<ComPtr<slang::IComponentType>, std::vector<slang::TypeLayoutReflection*>> Material::compileMaterialProgram(const Slang::ComPtr<slang::IModule>& materialModule,
                                                                                                                     slang::TypeReflection* materialType,
                                                                                                                     const SlangCompiler& compiler)
{
	auto rasterModule{compiler.loadModule("Core/mainRaster")};
	auto vertEntry{SlangCompiler::findEntryPoint(rasterModule, "vertexMain")};
	auto fragEntry{SlangCompiler::findEntryPoint(rasterModule, "fragmentMain")};

	auto lightModule{compiler.loadModule("Core/lights")};
	auto lightType{lightModule->getLayout()->findTypeByName(UniversalLightEnvironment::getLightTypeNameStatic().c_str())};

	auto composedProgram{compiler.composeProgram({rasterModule, vertEntry, fragEntry, materialModule, lightModule})};

	// TODO: Try to specialize by type
	std::array specializationArgs
	{
		slang::SpecializationArg{
			slang::SpecializationArg::Kind::Type,
			lightType
		},
		slang::SpecializationArg{
			slang::SpecializationArg::Kind::Type,
			materialType
		},
	};

	auto program{SlangCompiler::specializeProgram(composedProgram, specializationArgs)};
	return {program, {program->getLayout()->getTypeLayout(lightType), program->getLayout()->getTypeLayout(materialType)}};
}

vk::raii::PipelineLayout Material::createPipelineLayout(const VulkanShaderObjectLayout& layout, const Renderer& app)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{{}, *layout.descriptorSetLayout, nullptr};

	return {app.device, pipelineLayoutCreateInfo};
}

vk::raii::Pipeline Material::createPipeline(const Spirv& spirv, const vk::raii::PipelineLayout& layout, const Renderer& app)
{
	vk::raii::ShaderModule vertShaderModule{createShaderModule(spirv.vertSpirv, app.device)};
	vk::raii::ShaderModule fragShaderModule{createShaderModule(spirv.fragSpirv, app.device)};

	vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main", nullptr};
	vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main", nullptr};

	// TODO: Move all of these parts somewhere else to make this code self-documenting
	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{vertShaderStageCreateInfo, fragShaderStageCreateInfo};

	auto bindingDescription{Vertex::getBindingDescription()};
	auto attributeDescriptions{Vertex::getAttributeDescriptions()};
	vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{{}, bindingDescription, attributeDescriptions};

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{{}, vk::PrimitiveTopology::eTriangleList, false};

	std::array<vk::DynamicState, 2> dynamicStates{
		vk::DynamicState::eViewportWithCount,
		vk::DynamicState::eScissorWithCount
	};
	vk::PipelineDynamicStateCreateInfo dynamicState{{}, dynamicStates};

	vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{{}, nullptr, nullptr}; // TODO: Counts were originally 1, so this might lead to problems

	vk::PipelineRasterizationStateCreateInfo rasterizer{
		{}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, false,
		0.f, 0.f, 0.f, 1.f
	};

	vk::PipelineMultisampleStateCreateInfo multisampleState{{}, vk::SampleCountFlagBits::e1, false, 1.f, nullptr, false, false};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		false, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};

	vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{{}, false, vk::LogicOp::eCopy, colorBlendAttachment, {0.f}};

	vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{{}, true, true, vk::CompareOp::eLessOrEqual, false, false, {}, {}, 0.f, 1.f};

	vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
		{}, shaderStages, &vertexInputStateCreateInfo, &inputAssemblyStateCreateInfo, nullptr, &viewportStateCreateInfo, &rasterizer, &multisampleState,
		&depthStencilInfo, &colorBlendStateCreateInfo, &dynamicState, layout, app.renderPass, 0, {}, -1
	};

	return {app.device, nullptr, pipelineCreateInfo};
}

vk::raii::ShaderModule Material::createShaderModule(const Slang::ComPtr<slang::IBlob>& codeBlob, const vk::raii::Device& device)
{
	const vk::ShaderModuleCreateInfo createInfo{{}, codeBlob->getBufferSize(), static_cast<const uint32_t*>(codeBlob->getBufferPointer())};
	return device.createShaderModule(createInfo);
}
