#pragma once

#include <stdexcept>
#include <vector>

#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

using Slang::ComPtr;

struct ExampleResources
{
    Slang::String baseDir;

    ExampleResources(const Slang::String& dir)
        : baseDir(dir)
    {
    }

    Slang::String resolveResource(const char* fileName) const
    {
        static const Slang::List<Slang::String> directories{
            "examples",
            "../examples",
            "../../examples",
        };

        for (const Slang::String& dir : directories)
        {
            Slang::StringBuilder pathSb;
            pathSb << dir << "/" << baseDir << "/" << fileName;
            if (Slang::File::exists(pathSb.getBuffer()))
                return pathSb.toString();
        }

        return fileName;
    }
};

static const ExampleResources resourceBase("Shaders");

inline SlangResult check(SlangResult result)
{
    if (result != SLANG_OK)
    {
        throw std::runtime_error("Check failed");
    }

    return result;
}

inline void compileShader()
{
    ComPtr<slang::IGlobalSession> slangGlobalSession;
    check(createGlobalSession(slangGlobalSession.writeRef())); // TODO: We should not create a new session every time

    slang::TargetDesc targetDesc{
        .format = SLANG_SPIRV,
        .profile = slangGlobalSession->findProfile("spirv_1_5")
    };
    
    std::vector<slang::CompilerOptionEntry> options{
        {
            .name= slang::CompilerOptionName::EmitSpirvDirectly, .value= {
                .kind = slang::CompilerOptionValueKind::Int, .intValue0 = 1, .intValue1 = 0, .stringValue0 = nullptr, .stringValue1 = nullptr
            }
        }
    };
    
    slang::SessionDesc sessionDesc{
        .targets = &targetDesc,
        .targetCount = 1,
        .compilerOptionEntries = options.data(),
        .compilerOptionEntryCount = static_cast<uint32_t>(options.size())
    };

    ComPtr<slang::ISession> session;
    check(slangGlobalSession->createSession(sessionDesc, session.writeRef()));

    slang::IModule* module;
    {
        ComPtr<slang::IBlob> diagnosticBlob;
        Slang::String path{resourceBase.resolveResource("default.slang")}; // TODO: This should be VERY different
        module = session->loadModule(path.getBuffer(), diagnosticBlob.writeRef());
        diagnoseIfNeeded(diagnosticBlob);
        if (!module)
        {
            throw std::runtime_error("Failed to load shader module");
        }

        ComPtr<slang::IEntryPoint> entryPoint;
        module->getDefinedEntryPoint(..., entryPoint.writeRef());
    }
}
