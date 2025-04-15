#pragma once
#include <slang/slang-com-ptr.h>

#include "AssetBase.hpp"
#include "ShaderCompilation/VulkanShaderObjectLayout.hpp"

struct Spirv
{
public:
	Slang::ComPtr<slang::IBlob> vertSpirv;
	Slang::ComPtr<slang::IBlob> fragSpirv;
};

class SlangCompiler;

class Material : public AssetBase
{
public:
	Material();

	void compile(const SlangCompiler& compiler, const Renderer& app);

	Spirv spirv;

	Slang::ComPtr<slang::IComponentType> program;

	std::shared_ptr<VulkanShaderObjectLayout> shaderLayout; // TODO: This all screams for a refactor that separates material assets from compiled materials
	vk::raii::PipelineLayout pipelineLayout;
	vk::raii::Pipeline pipeline;

private:
	static std::pair<Slang::ComPtr<slang::IModule>, slang::TypeReflection*> loadMaterial(const std::string_view& materialModuleName, const std::string_view& materialType,
	                                                                                     const SlangCompiler& compiler);
	static Spirv compileSpirv(const Slang::ComPtr<slang::IComponentType>& program);
	static Slang::ComPtr<slang::IComponentType> compileMaterialProgram(const Slang::ComPtr<slang::IModule>& materialModule, slang::TypeReflection* materialType, const SlangCompiler& compiler);
	static vk::raii::PipelineLayout createPipelineLayout(const VulkanShaderObjectLayout& layout, const Renderer& app);
	static vk::raii::Pipeline createPipeline(const Spirv &spirv, const vk::raii::PipelineLayout& layout, const Renderer& app);
	static vk::raii::ShaderModule createShaderModule(const Slang::ComPtr<slang::IBlob>& codeBlob, const vk::raii::Device& device);
};
