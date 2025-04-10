//
// Created by Thomas on 10.04.2025.
//

#include "SlangDebug.hpp"

SlangDebug::SlangPrinter & SlangDebug::SlangPrinter::operator<<(slang::VariableReflection *var) {
    return *this << "name: \"" << var->getName() << "\"\n" << PrintIndent{} << "type:\n" << BeginIndent{} <<
        PrintIndent{} << var->getType() << EndIndent{};
}

SlangDebug::SlangPrinter & SlangDebug::SlangPrinter::operator<<(slang::TypeReflection *type) {
    *this << "name: \"" << type->getName() << "\"\n" << PrintIndent{} << "kind: " << type->getKind();

    switch (type->getKind()) {
        default:
        case slang::TypeReflection::Kind::None:
            break;
        case slang::TypeReflection::Kind::Struct:
            printStructType(type);
        case slang::TypeReflection::Kind::Array:
            break;
        case slang::TypeReflection::Kind::Matrix:
            break;
        case slang::TypeReflection::Kind::Vector:
            break;
        case slang::TypeReflection::Kind::Scalar:
            printScalarType(type);
        case slang::TypeReflection::Kind::ConstantBuffer:
            break;
        case slang::TypeReflection::Kind::Resource:
            break;
        case slang::TypeReflection::Kind::SamplerState:
            break;
        case slang::TypeReflection::Kind::TextureBuffer:
            break;
        case slang::TypeReflection::Kind::ShaderStorageBuffer:
            break;
        case slang::TypeReflection::Kind::ParameterBlock:
            break;
        case slang::TypeReflection::Kind::GenericTypeParameter:
            break;
        case slang::TypeReflection::Kind::Interface:
            break;
        case slang::TypeReflection::Kind::OutputStream:
            break;
        case slang::TypeReflection::Kind::Specialized:
            break;
        case slang::TypeReflection::Kind::Feedback:
            break;
        case slang::TypeReflection::Kind::Pointer:
            break;
        case slang::TypeReflection::Kind::DynamicResource:
            break;
    }
    return *this;
}

SlangDebug::SlangPrinter &SlangDebug::SlangPrinter::operator<<(slang::TypeReflection::Kind kind) {
    switch (kind) {
        case slang::TypeReflection::Kind::None:
            return *this << "none";
        case slang::TypeReflection::Kind::Struct:
            return *this << "struct";
        case slang::TypeReflection::Kind::Array:
            return *this << "array";
        case slang::TypeReflection::Kind::Matrix:
            return *this << "matrix";
        case slang::TypeReflection::Kind::Vector:
            return *this << "vector";
        case slang::TypeReflection::Kind::Scalar:
            return *this << "scalar";
        case slang::TypeReflection::Kind::ConstantBuffer:
            return *this << "constant buffer";
        case slang::TypeReflection::Kind::Resource:
            return *this << "resource";
        case slang::TypeReflection::Kind::SamplerState:
            return *this << "sampler state";
        case slang::TypeReflection::Kind::TextureBuffer:
            return *this << "texture buffer";
        case slang::TypeReflection::Kind::ShaderStorageBuffer:
            return *this << "shader storage buffer";
        case slang::TypeReflection::Kind::ParameterBlock:
            return *this << "parameter block";
        case slang::TypeReflection::Kind::GenericTypeParameter:
            return *this << "generic type parameter";
        case slang::TypeReflection::Kind::Interface:
            return *this << "interface";
        case slang::TypeReflection::Kind::OutputStream:
            return *this << "output stream";
        case slang::TypeReflection::Kind::Specialized:
            return *this << "specialized";
        case slang::TypeReflection::Kind::Feedback:
            return *this << "feedback";
        case slang::TypeReflection::Kind::Pointer:
            return *this << "pointer";
        case slang::TypeReflection::Kind::DynamicResource:
            return *this << "dynamic resource";
    }
    return *this;
}

SlangDebug::SlangPrinter & SlangDebug::SlangPrinter::operator<<(slang::TypeReflection::ScalarType type) {
    switch (type) {
        case slang::TypeReflection::None:
            return *this << "none";
        case slang::TypeReflection::Void:
            return *this << "void";
        case slang::TypeReflection::Bool:
            return *this << "bool";
        case slang::TypeReflection::Int32:
            return *this << "int32";
        case slang::TypeReflection::UInt32:
            return *this << "uint32";
        case slang::TypeReflection::Int64:
            return *this << "int64";
        case slang::TypeReflection::UInt64:
            return *this << "uint64";
        case slang::TypeReflection::Float16:
            return *this << "float16";
        case slang::TypeReflection::Float32:
            return *this << "float32";
        case slang::TypeReflection::Float64:
            return *this << "float64";
        case slang::TypeReflection::Int8:
            return *this << "int8";
        case slang::TypeReflection::UInt8:
            return *this << "uint8";
        case slang::TypeReflection::Int16:
            return *this << "int16";
        case slang::TypeReflection::UInt16:
            return *this << "uint16";
    }
    return *this;
}

SlangDebug::SlangPrinter & SlangDebug::SlangPrinter::operator<<(BeginIndent) {
    ++indent;
    return *this;
}

SlangDebug::SlangPrinter & SlangDebug::SlangPrinter::operator<<(EndIndent) {
    --indent;
    return *this;
}

SlangDebug::SlangPrinter & SlangDebug::SlangPrinter::operator<<(PrintIndent) {
    for (int i = 0; i < indent; ++i) {
        s << "\t";
    }
    return *this;
}

void SlangDebug::SlangPrinter::printScalarType(slang::TypeReflection *type) {
    *this << '\n' << BeginIndent{} << PrintIndent{} << "scalar type: " << type->getScalarType() << EndIndent{};
}

void SlangDebug::SlangPrinter::printStructType(slang::TypeReflection *type) {
    *this << '\n' << BeginIndent{} << PrintIndent{} << "fields:" << BeginIndent{};
    for (int i = 0; i < type->getFieldCount(); ++i) {
        *this << '\n' << PrintIndent{} << type->getFieldByIndex(i);
    }
    *this << EndIndent{} << EndIndent{};
}

std::ostream & SlangDebug::operator<<(std::ostream &os, slang::VariableReflection *var) {
    return os << "name: \"" << var->getName() << "\"\ntype: " << var->getType();
}

std::ostream & SlangDebug::operator<<(std::ostream &os, slang::TypeReflection *type) {
    return os << "name: \"" << type->getName() << "\"\nkind: " << type->getKind();
}
