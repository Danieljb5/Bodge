#pragma once

#include "backend.hpp"

struct CBackendGenerator : public BackendGenerator
{
    void GenerateFunctionPrototype(const std::string& name, std::vector<Value> args, Type ret, bool mangle = true, bool hasScope = false) override;
    void GenerateMemberFunctionPrototype(const std::string& name, std::vector<Value> args, Type ret, bool hasScope = false) override;
    void GenerateStruct(const std::string& name) override;
    void GenerateEnum(const std::string& name) override;
    void GenerateStructScopeStart() override;
    void GenerateStructScopeEnd() override;
    void GenerateScopeStart(bool inScope = false) override;
    void GenerateScopeEnd(bool inScope = false) override;

    void GenerateInclude(const std::string& filename) override;

    void GenerateVariableDeclaration(const Value& val) override;
    void GenerateVariableDeclarationWithAssignment(const Value& val) override;
    void GenerateMemberVariableDeclaration(const Value& val) override;

    void GenerateScopedVariableDeclaration(const Value& val) override;
    void GenerateScopedVariableAssignment(const std::string& val, const std::string& assignment = "=") override;
    void GenerateScopedVariableDeclarationWithAssignment(const Value& val) override;
    void BeginFunctionCall(const std::string& func) override;
    void GenerateFunctionCall(const std::vector<Type>& values, std::vector<std::pair<std::string, bool>> namespaces) override;
    void GenerateIf() override;
    void GenerateElse() override;
    void GenerateElseIf() override;
    void GenerateWhile() override;
    void GenerateBreak() override;
    void GenerateContinue() override;
    void GenerateReturn() override;
    void GenerateString(const std::string& str, bool inScope = false) override;
    void GenerateLineEnd(bool inScope = false) override;

    void PushNamespaceStack(const std::string& name, bool isStruct = false) override;
    void PopNamespaceStack() override;

    std::string Mangle(const std::string& name, const std::vector<Type>& types = {}) override;
    std::string Mangle(const std::string& name, const std::vector<Value>& types) override;

private:
    size_t lastMemberIndex = 0;
    size_t headerLastMemberIndex = 0;
    std::string structName = "";
    std::string indent = "";
    std::string funcNameCache = "";
};