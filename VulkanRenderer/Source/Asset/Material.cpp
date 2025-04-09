﻿//
// Created by Thomas on 30/03/2025.
//

#include "Material.hpp"

#include <array>

#include "Renderer.hpp"
#include "ShaderCompiler.hpp"
#include "Vertex.hpp"

Material::Material()
	: pipelineLayout(nullptr), pipeline(nullptr)
{
}

void Material::compile(const SlangCompiler& compiler, const Renderer& app)
{
	auto [materialModule, materialType]{loadMaterial("BRDF/phong", "ConstantPhongMaterial", compiler)};
	spirv = compileSpriv(materialModule, materialType, compiler);
	shaderLayout = std::make_unique<VulkanShaderObjectLayout>(materialModule->getLayout()->getTypeLayout(materialType), app);
	pipelineLayout = createPipelineLayout(*shaderLayout, app);
	pipeline = createPipeline(spirv, pipelineLayout, app);
}

std::pair<Slang::ComPtr<slang::IModule>, slang::TypeReflection*> Material::loadMaterial(const std::string_view& materialModuleName, const std::string_view& materialType, const SlangCompiler& compiler)
{
	auto materialModule{compiler.loadModule(materialModuleName)};
	auto material{materialModule->getLayout()->findTypeByName(materialType.data())};
	return {materialModule, material};
}

Slang::ComPtr<slang::IBlob> Material::compileSpriv(const Slang::ComPtr<slang::IModule>& materialModule, slang::TypeReflection* materialType, const SlangCompiler& compiler)
{
	auto rasterModule{compiler.loadModule("Core/mainRaster")};
	auto vertEntry{SlangCompiler::findEntryPoint(rasterModule, "vertexMain")};
	auto fragEntry{SlangCompiler::findEntryPoint(rasterModule, "fragmentMain")};

	auto lightModule{compiler.loadModule("Core/lights")};
	auto lightType{lightModule->getLayout()->findTypeByName("DirectionalLight")};

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
	auto specializedCode{SlangCompiler::specializeProgram(composedProgram, specializationArgs)};

	return SlangCompiler::getSprirV(SlangCompiler::linkProgram(specializedCode));
}

vk::raii::PipelineLayout Material::createPipelineLayout(const VulkanShaderObjectLayout& layout, const Renderer& app)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{{}, *layout.descriptorSetLayout, nullptr};

	return {app.device, pipelineLayoutCreateInfo};
}

vk::raii::Pipeline Material::createPipeline(const Slang::ComPtr<slang::IBlob>& spirv, const vk::raii::PipelineLayout& layout, const Renderer& app)
{
	vk::raii::ShaderModule shaderModule{createShaderModule(spirv, app.device)};

	vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, shaderModule, "vertexMain", nullptr};
	vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eFragment, shaderModule, "fragmentMain", nullptr};

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
