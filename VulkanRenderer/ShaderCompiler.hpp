#pragma once

#include <filesystem>
#include <span>
#include <stdexcept>
#include <vector>

#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

using Slang::ComPtr;

class SlangCompiler
{
public:

	SlangCompiler();

	[[nodiscard]] ComPtr<slang::IModule> loadModule(const std::string_view& moduleName) const;
	[[nodiscard]] static ComPtr<slang::IEntryPoint> findEntryPoint(const ComPtr<slang::IModule>& module, const std::string_view& entryPointName);
	[[nodiscard]] ComPtr<slang::IComponentType> composeProgram(const std::vector<slang::IComponentType*>& components) const;
	[[nodiscard]] static ComPtr<slang::IComponentType> linkProgram(const ComPtr<slang::IComponentType>& composedProgram);
	[[nodiscard]] static ComPtr<slang::IBlob> getSprirV(const ComPtr<slang::IComponentType>& linkedProgram, uint32_t entryPointIndex);
	[[nodiscard]] ComPtr<slang::IBlob> compile(const std::string_view& moduleName, const std::string_view& entryPointName) const;
	[[nodiscard]] static ComPtr<slang::IComponentType> specializeEntryPoint(const ComPtr<slang::IEntryPoint>& entryPoint, const std::span<slang::SpecializationArg>& specializationArgs);
	[[nodiscard]] static ComPtr<slang::IComponentType> specializeProgram(const ComPtr<slang::IComponentType>& program, const std::span<slang::SpecializationArg>& specializationArgs);

private:
	ComPtr<slang::IGlobalSession> globalSession;
	slang::TargetDesc targetDesc;
	std::vector<slang::CompilerOptionEntry> options;
	slang::SessionDesc sessionDesc;
	ComPtr<slang::ISession> session;

	static ComPtr<slang::IGlobalSession> createGlobalSession();
	static ComPtr<slang::ISession> createSession(const ComPtr<slang::IGlobalSession>& globalSession, const slang::SessionDesc& sessionDesc);

	static void diagnoseIfNeeded(const ComPtr<slang::IBlob>& diagnosticsBlob);

	template<typename T>
	static ComPtr<T> check(const ComPtr<T>& Value)
	{
		if (!Value)
		{
			throw std::runtime_error("Check failed");
		}
		return Value;
	}

	static SlangResult check(const SlangResult result)
	{
		if (result != SLANG_OK)
		{
			throw std::runtime_error("Check failed");
		}

		return result;
	}
};