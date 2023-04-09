#include "cbackend.hpp"

void CBackendGenerator::GenerateFunctionPrototype(const std::string& name, std::vector<Value> args, Type ret, bool mangle, bool hasScope)
{
    std::string name_;
    if(mangle) name_ = Mangle(name, args);
    else name_ = name;

    if(publicEnabled)
    {
        headerOutput.push_back({"", false});
        std::string& out = headerOutput.back().first;

        out += DeconstructType(ret) + " ";
        out += name_ + "(";

        for(size_t i = 0; i < args.size(); i++)
        {
            knownVariables[args[i].name] = args[i].type;
            out += DeconstructType(args[i].type) + " " + args[i].name;
            if(i + 1 < args.size())
            {
                out += ", ";
            }
        }

        out += ")";
    }
    if(!publicEnabled || hasScope)
    {
        output.push_back({"", false});
        std::string& out = output.back().first;

        out += DeconstructType(ret) + " ";
        out += name_ + "(";

        for(size_t i = 0; i < args.size(); i++)
        {
            knownVariables[args[i].name] = args[i].type;
            out += DeconstructType(args[i].type) + " " + args[i].name;
            if(i + 1 < args.size())
            {
                out += ", ";
            }
        }

        out += ")";
    }

    knownFunctions[name_] = ret;
}

void CBackendGenerator::GenerateMemberFunctionPrototype(const std::string& name, std::vector<Value> args, Type ret, bool hasScope)
{
    auto name_ = Mangle(name, args);

    if(publicEnabled)
    {
        headerOutput.push_back({"", false});
        std::string& out = headerOutput.back().first;

        out += DeconstructType(ret) + " ";
        out += name_ + "(";
        out += structName + "* this";

        for(size_t i = 0; i < args.size(); i++)
        {
            out += ", " + DeconstructType(args[i].type) + " " + args[i].name;
        }

        out += ")";
    }
    if(!publicEnabled || hasScope)
    {
        output.push_back({"", false});
        std::string& out = output.back().first;

        out += DeconstructType(ret) + " ";
        out += name_ + "(" + structName + "* this";

        for(size_t i = 0; i < args.size(); i++)
        {
            out += ", ";
            out += DeconstructType(args[i].type) + " " + args[i].name;
        }

        out += ")";
    }

    knownFunctions[name_] = ret;
}

void CBackendGenerator::GenerateStruct(const std::string& name)
{
    structName = Mangle(name);
    if(publicEnabled)
    {
        headerLastMemberIndex = headerOutput.size();
        headerOutput.push_back({"", false});
        std::string& out = headerOutput.back().first;
        out += "struct " + structName + "_";
        headerOutput.push_back({"", false});
        std::string& out_ = headerOutput.back().first;
        out_ += "typedef struct " + structName + "_ " + structName;
    }
    else
    {
        lastMemberIndex = output.size();
        output.push_back({"", false});
        std::string& out = output.back().first;

        out += "struct " + structName + "_";
        output.push_back({"", false});
        std::string& out_ = output.back().first;
        out_ += "typedef struct " + structName + "_ " + structName;
    }
    typeIDs.insert({structName, Type::StructID});
}

void CBackendGenerator::GenerateEnum(const std::string& name)
{
    if(publicEnabled)
    {
        headerOutput.push_back({"", false});
        std::string& out = headerOutput.back().first;
        out += "enum " + name;
    }
    else
    {
        output.push_back({"", false});
        std::string& out = output.back().first;
        out += "enum " + name;
    }

}

void CBackendGenerator::GenerateStructScopeStart()
{
    if(publicEnabled)
    {
        headerOutput[headerLastMemberIndex].first += "\n{\n";
    }
    else
    {
        output[lastMemberIndex].first += "\n{\n";
    }
    indent += "    ";

}

void CBackendGenerator::GenerateStructScopeEnd()
{
    if(publicEnabled)
    {
        headerOutput[headerLastMemberIndex].first += "\n};";
        headerOutput[headerLastMemberIndex].second = true;
    }
    else
    {
        output[lastMemberIndex].first += "\n};";
        output[lastMemberIndex].second = true;
    }
    for(int i = 0; i < 4; i++) indent.pop_back();
}

void CBackendGenerator::GenerateScopeStart(bool inScope)
{
    if(publicEnabled && !inScope)
    {
        headerOutput.back().first += "\n" + indent + "{\n";
    }
    else
    {
        output.back().first += "\n" + indent + "{\n";
    }
    indent += "    ";
}

void CBackendGenerator::GenerateScopeEnd(bool inScope)
{
    for(int i = 0; i < 4; i++) indent.pop_back();
    if(publicEnabled && !inScope)
    {
        headerOutput.back().first += "\n" + indent + "}\n";
        headerOutput.back().second = true;
    }
    else
    {
        output.back().first += "\n" + indent + "}\n";
        output.back().second = true;
    }
}


void CBackendGenerator::GenerateInclude(const std::string& filename)
{
    if(publicEnabled)
    {
        headerOutput.push_back({"", true});
        std::string& out = headerOutput.back().first;
        out += "#include " + filename;
    }
    else
    {
        output.push_back({"", true});
        std::string& out = output.back().first;
        out += "#include " + filename;
    }
}


void CBackendGenerator::GenerateVariableDeclaration(const Value& val)
{
    if(publicEnabled)
    {
        headerOutput.push_back({"", false});
        std::string& out = headerOutput.back().first;

        out += DeconstructType(val.type) + " " + val.name;
    }
    else
    {
        output.push_back({"", false});
        std::string& out = output.back().first;

        out += DeconstructType(val.type) + " " + val.name;
    }
    knownVariables.insert({val.name, val.type});
}

void CBackendGenerator::GenerateVariableDeclarationWithAssignment(const Value& val)
{
    if(publicEnabled)
    {
        headerOutput.push_back({"", false});
        std::string& out = headerOutput.back().first;

        out += DeconstructType(val.type) + " " + val.name + " = ";
    }
    else
    {
        output.push_back({"", false});
        std::string& out = output.back().first;

        out += DeconstructType(val.type) + " " + val.name + " = ";
    }
    knownVariables.insert({val.name, val.type});
}

void CBackendGenerator::GenerateMemberVariableDeclaration(const Value& val)
{
    if(publicEnabled)
    {
        std::string& out = headerOutput[headerLastMemberIndex].first;

        out += indent + DeconstructType(val.type) + " " + val.name + ";\n";
        knownVariables.insert({Mangle(val.name), val.type});
    }
    else
    {
        std::string& out = output[lastMemberIndex].first;

        out += indent + DeconstructType(val.type) + " " + val.name + ";\n";
        knownVariables.insert({Mangle(val.name), val.type});
    }
}


void CBackendGenerator::GenerateScopedVariableDeclaration(const Value& val)
{
    std::string& out = output.back().first;

    out += indent + DeconstructType(val.type) + " " + val.name + ";\n";
    knownVariables.insert({val.name, val.type});
}

void CBackendGenerator::GenerateScopedVariableAssignment(const std::string& val, const std::string& assignment)
{
    std::string& out = output.back().first;

    out += indent + val + " " + assignment + " ";
}

void CBackendGenerator::GenerateScopedVariableDeclarationWithAssignment(const Value& val)
{
    std::string& out = output.back().first;

    out += indent + DeconstructType(val.type) + " " + val.name + " = ";
    knownVariables.insert({val.name, val.type});
}

void CBackendGenerator::BeginFunctionCall(const std::string& func)
{
    funcNameCache = func;
    output.push_back({"", true});
}

void CBackendGenerator::GenerateFunctionCall(const std::vector<Type>& values, std::vector<std::pair<std::string, bool>> namespaces)
{
    auto mangledName = GenerateMangledFunction(namespaces, funcNameCache, values);
    auto namespaceName = GenerateMangledNamespace(namespaces);
    auto args = output.back().first;
    if(!namespaces.empty() && namespaces.front().first == "c")
    {
        mangledName = funcNameCache;
    }
    if(IsMemberNamespace(namespaces))
    {
        std::string args_;
        if(namespaces.size() > 1) args_ = GenerateMangledNamespace(namespaces);
        else args_ = "&" + namespaces.back().first;
        if(args != "") args = ", " + args;
        args = args_ + args;
        namespaces.back().first = GetType(namespaces.back().first).structName;
        mangledName = GenerateMangledFunction(namespaces, funcNameCache, values);
        namespaceName = GenerateMangledNamespace(namespaces);
    }
    output.pop_back();

    std::string& out = output.back().first;
    out += indent + mangledName + "(" + args + ")";
}

void CBackendGenerator::GenerateIf()
{
    std::string& out = output.back().first;

    out += indent + "if(";
}

void CBackendGenerator::GenerateElse()
{
    std::string& out = output.back().first;

    out += indent + "else";
}

void CBackendGenerator::GenerateElseIf()
{
    std::string& out = output.back().first;

    out += indent + "else if(";
}

void CBackendGenerator::GenerateWhile()
{
    std::string& out = output.back().first;

    out += indent + "while(";
}

void CBackendGenerator::GenerateBreak()
{
    std::string& out = output.back().first;

    out += indent + "break;\n";
}

void CBackendGenerator::GenerateContinue()
{
    std::string& out = output.back().first;

    out += indent + "continue;\n";
}

void CBackendGenerator::GenerateReturn()
{
    std::string& out = output.back().first;

    out += indent + "return ";
}


void CBackendGenerator::GenerateString(const std::string& str, bool inScope)
{
    if(publicEnabled && !inScope)
    {
        std::string& out = headerOutput.back().first;

        out += str;
    }
    else
    {
        std::string& out = output.back().first;

        out += str;
    }
}

void CBackendGenerator::GenerateLineEnd(bool inScope)
{
    if(publicEnabled && !inScope)
    {
        std::string& out = headerOutput.back().first;

        out += ";\n";
    }
    else
    {
        std::string& out = output.back().first;

        out += ";\n";
    }
}


void CBackendGenerator::PushNamespaceStack(const std::string& name, bool isStruct)
{
    namespaceStack.push_back({name, false});
    if(!isStruct) knownNamespaces.insert(GenerateMangledNamespace(namespaceStack));
}

void CBackendGenerator::PopNamespaceStack()
{
    namespaceStack.pop_back();
}


std::string CBackendGenerator::Mangle(const std::string& name, const std::vector<Type>& types, bool isVar)
{
    std::string result = "";
    bool isMember = false;
    for(size_t i = 0; i < namespaceStack.size(); i++)
    {
        auto& ns = namespaceStack[i];
        if(ns.first == "") continue;
        if(ns.second && isVar)
        {
            isMember = true;
            result += ns.first + ".";
        }
        else
        {
            if(ns.first[0] == '_' && isDigit(ns.first[1]))
            {   // already mangled
                result += ns.first;
            }
            else
            {
                result += std::to_string(ns.first.length());
                result += ns.first;
                if(i + 1 < namespaceStack.size())
                {
                    auto& ns2 = namespaceStack[i + 1];
                    if(ns2.first[0] != '_' && !isDigit(ns2.first[1]))
                    {
                        result += "_";
                    }
                }
            }
        }
    }
    if(!isMember) result += "_" + std::to_string(name.length());
    result += name;
    for(auto& t : types)
    {
        result += "_";
        result += MangleType(t);
    }
    if(isMember) return result;
    if(namespaceStack.front().first[0] == '_' && isDigit(namespaceStack.front().first[1])) return result;
    return "_" + result;
}

std::string CBackendGenerator::Mangle(const std::string& name, const std::vector<Value>& types, bool isVar)
{
    std::string result = "";
    bool isMember = false;
    for(auto& ns : namespaceStack)
    {
        if(ns.second && isVar)
        {
            isMember = true;
            result += ns.first + ".";
        }
        else
        {
            result += std::to_string(ns.first.length());
            result += ns.first;
            result += "_";
        }
    }
    if(!isMember) result += std::to_string(name.length());
    result += name;
    for(auto& t : types)
    {
        result += "_";
        result += MangleType(t.type);
    }
    if(isMember) return result;
    return "_" + result;
}
