#pragma once
#include <ostream>
#include <sstream>
#include <slang/slang.h>

namespace SlangDebug {

    struct SlangPrinter {
        SlangPrinter& operator<<(slang::VariableReflection* var);
        SlangPrinter& operator<<(slang::TypeReflection* type);
        SlangPrinter& operator<<(slang::TypeReflection::Kind kind);
        SlangPrinter& operator<<(slang::TypeReflection::ScalarType type);

        struct BeginIndent{};
        struct EndIndent{};
        struct PrintIndent{};

        SlangPrinter& operator<<(BeginIndent);
        SlangPrinter& operator<<(EndIndent);
        SlangPrinter& operator<<(PrintIndent);

        template<typename T>
        SlangPrinter& operator<<(const T& t) {
            s << t;
            return *this;
        }

    private:
        int indent{0};
        std::stringstream s;

        void printScalarType(slang::TypeReflection* type);
        void printStructType(slang::TypeReflection* type);
    };

    std::ostream& operator<<(std::ostream& os, slang::VariableReflection* var);
    std::ostream& operator<<(std::ostream& os, slang::TypeReflection* type);
}
