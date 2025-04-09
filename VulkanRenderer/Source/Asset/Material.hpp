#pragma once
#include <slang/slang-com-ptr.h>

#include "ShaderCompilation/VulkanShaderObjectLayout.hpp"

class SlangCompiler;

class Material
{
public:
	Material();

	void compile(const SlangCompiler& compiler, const Renderer& app);

	Slang::ComPtr<slang::IBlob> spirv;

	std::unique_ptr<VulkanShaderObjectLayout> shaderLayout; // TODO: This all screams for a refactor that separates material assets from compiled materials
	vk::raii::PipelineLayout pipelineLayout;
	vk::raii::Pipeline pipeline;

private:
	static std::pair<Slang::ComPtr<slang::IModule>, slang::TypeReflection*> loadMaterial(const std::string_view& materialModuleName, const std::string_view& materialType,
	                                                                                     const SlangCompiler& compiler);
	static Slang::ComPtr<slang::IBlob> compileSpriv(const Slang::ComPtr<slang::IModule>& materialModule, slang::TypeReflection* materialType, const SlangCompiler& compiler);
	static vk::raii::PipelineLayout createPipelineLayout(const VulkanShaderObjectLayout& layout, const Renderer& app);
	static vk::raii::Pipeline createPipeline(const Slang::ComPtr<slang::IBlob>& spirv, const vk::raii::PipelineLayout& layout, const Renderer& app);
	static vk::raii::ShaderModule createShaderModule(const Slang::ComPtr<slang::IBlob>& codeBlob, const vk::raii::Device& device);
};
