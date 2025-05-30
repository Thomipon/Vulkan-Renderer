﻿#include "ShaderCompiler.hpp"

#include <array>
#include <iostream>

#include "slang/slang-com-helper.h"

static std::array<const char*, 1> baseShaderPaths{"../../VulkanRenderer/Shaders"}; // TODO: This should not be hardcoded

SlangCompiler::SlangCompiler()
	: globalSession(createGlobalSession()),
	  targetDesc{
		  .format = SLANG_SPIRV,
		  .profile = globalSession->findProfile("spirv_1_5")
	  },
	  options{
		  slang::CompilerOptionEntry{
			  .name = slang::CompilerOptionName::EmitSpirvDirectly,
			  .value = {
				  .kind = slang::CompilerOptionValueKind::Int, .intValue0 = 1, .intValue1 = 0, .stringValue0 = nullptr, .stringValue1 = nullptr
			  }
		  },
		  slang::CompilerOptionEntry{
			  .name = slang::CompilerOptionName::MatrixLayoutColumn,
			  .value = {
				  .kind = slang::CompilerOptionValueKind::Int, .intValue0 = 1, .intValue1 = 1, .stringValue0 = nullptr, .stringValue1 = nullptr
			  }
		  }
	  },
	  sessionDesc{
		  .targets = &targetDesc,
		  .targetCount = 1,
		  .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
		  .searchPaths = baseShaderPaths.data(),
		  .searchPathCount = static_cast<uint32_t>(baseShaderPaths.size()),
		  .compilerOptionEntries = options.data(),
		  .compilerOptionEntryCount = static_cast<uint32_t>(options.size())
	  },
	  session(createSession(globalSession, sessionDesc))
{
}

ComPtr<slang::IModule> SlangCompiler::loadModule(const std::string_view& moduleName) const
{
	ComPtr<slang::IModule> module;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		module = session->loadModule(moduleName.data(), diagnosticsBlob.writeRef());
		diagnoseIfNeeded(diagnosticsBlob);
		check(module);
	}
	return module;
}

ComPtr<slang::IEntryPoint> SlangCompiler::findEntryPoint(const ComPtr<slang::IModule>& module, const std::string_view& entryPointName)
{
	// TODO: We would want to use an index instead of name ideally
	ComPtr<slang::IEntryPoint> entryPoint;
	{
		module->findEntryPointByName(entryPointName.data(), entryPoint.writeRef());
		check(entryPoint);
	}
	return entryPoint;
}

ComPtr<slang::IComponentType> SlangCompiler::composeProgram(const std::vector<slang::IComponentType*>& components) const
{
	ComPtr<slang::IComponentType> composedProgram;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		check(session->createCompositeComponentType(components.data(), static_cast<SlangInt>(components.size()), composedProgram.writeRef(), diagnosticsBlob.writeRef()));
		diagnoseIfNeeded(diagnosticsBlob);
	}
	return composedProgram;
}

ComPtr<slang::IComponentType> SlangCompiler::linkProgram(const ComPtr<slang::IComponentType>& composedProgram)
{
	ComPtr<slang::IComponentType> linkedProgram;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		const auto result{composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef())};
		diagnoseIfNeeded(diagnosticsBlob);
		check(result);
	}
	return linkedProgram;
}

ComPtr<slang::IBlob> SlangCompiler::getSprirV(const ComPtr<slang::IComponentType>& linkedProgram, const uint32_t entryPointIndex)
{
	ComPtr<slang::IBlob> spirvCode;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		const auto result = linkedProgram->getEntryPointCode(entryPointIndex, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
		diagnoseIfNeeded(diagnosticsBlob);
		check(result);
	}
	return spirvCode;
}

ComPtr<slang::IBlob> SlangCompiler::compile(const std::string_view& moduleName, const std::string_view& entryPointName) const
{
	const ComPtr<slang::IModule> module{loadModule(moduleName)};
	const ComPtr<slang::IEntryPoint> entryPoint{findEntryPoint(module, entryPointName)};
	return getSprirV(linkProgram(composeProgram({module, entryPoint})), 0);
}

ComPtr<slang::IComponentType> SlangCompiler::specializeEntryPoint(const ComPtr<slang::IEntryPoint>& entryPoint, const std::span<slang::SpecializationArg>& specializationArgs)
{
	ComPtr<slang::IComponentType> specializedEntryPoint;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		const auto result = entryPoint->specialize(specializationArgs.data(), specializationArgs.size(), specializedEntryPoint.writeRef(), diagnosticsBlob.writeRef());
		diagnoseIfNeeded(diagnosticsBlob);
		check(result);
	}
	return specializedEntryPoint;
}

ComPtr<slang::IComponentType> SlangCompiler::specializeProgram(const ComPtr<slang::IComponentType>& program, const std::span<slang::SpecializationArg>& specializationArgs)
{
	ComPtr<slang::IComponentType> specializedProgram;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		const auto result = program->specialize(specializationArgs.data(), specializationArgs.size(), specializedProgram.writeRef(), diagnosticsBlob.writeRef());
		diagnoseIfNeeded(diagnosticsBlob);
		check(result);
	}
	return specializedProgram;
}

slang::ProgramLayout* SlangCompiler::getProgramLayout(const ComPtr<slang::IComponentType>& program, int targetIndex)
{
	slang::ProgramLayout* programLayout{nullptr};
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		programLayout = program->getLayout(targetIndex, diagnosticsBlob.writeRef());
		diagnoseIfNeeded(diagnosticsBlob);
	}
	return programLayout;
}

ComPtr<slang::IGlobalSession> SlangCompiler::createGlobalSession()
{
	ComPtr<slang::IGlobalSession> session;
	check(slang::createGlobalSession(session.writeRef()));
	return session;
}

ComPtr<slang::ISession> SlangCompiler::createSession(const ComPtr<slang::IGlobalSession>& globalSession, const slang::SessionDesc& sessionDesc)
{
	ComPtr<slang::ISession> session;
	check(globalSession->createSession(sessionDesc, session.writeRef()));
	return session;
}

void SlangCompiler::diagnoseIfNeeded(const ComPtr<slang::IBlob>& diagnosticsBlob)
{
	if (diagnosticsBlob != nullptr)
	{
		std::cout << static_cast<const char*>(diagnosticsBlob->getBufferPointer()) << std::endl;
	}
}
