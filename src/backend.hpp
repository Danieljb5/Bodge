#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

struct Type
{
    enum TypeID
    {
        ERRID,      // error (type does not exist)
        FP32ID,     // 32-bit float
        FP64ID,     // 64-bit float
        FP128ID,    // 128-bit float
        VoidID,     // type with no size
        I8SID,      // signed 8-bit integer
        I16SID,     // signed 16-bit integer
        I32SID,     // signed 32-bit integer
        I64SID,     // signed 64-bit integer
        I128SID,    // signed 128-bit integer
        I8UID,      // unsigned 8-bit integer
        I16UID,     // unsigned 16-bit integer
        I32UID,     // unsigned 32-bit integer
        I64UID,     // unsigned 64-bit integer
        I128UID,    // unsigned 128-bit integer
        I1ID,       // boolean (1-bit integer)
        PointerID,
        StructID,
        ArrayID,
        UNDEFID,
    } id;

    Type(TypeID id, std::string structName = "")
    {
        this->id = id;
        this->structName = structName;
    }

    Type()
    {

    }

    Type(const Type& other)
    {
        id = other.id;
        structName = other.structName;
        if(other.next != nullptr)
        {
            next = new Type(*other.next);
        }
    }

    Type& operator=(const Type& other)
    {
        if(this != &other)
        {
            id = other.id;
            structName = other.structName;
            if(other.next != nullptr)
            {
                next = new Type(*other.next);
            }
        }
        return *this;
    }

    ~Type()
    {
        if(next)
        {
            delete next;
        }
    }

    std::string structName = "";
    Type* next = nullptr;
};

struct Value
{
    std::string name;
    Type type;
};

bool isDigit(char c);

struct BackendGenerator
{
    std::pair<std::string, std::string> Finish()
    {
        std::pair<std::string, std::string> result = {"", ""};
        for(auto& str : output)
        {
            if(!str.second) str.first += ";";
            result.first += "\n" + str.first;
        }
        for(auto& str : headerOutput)
        {
            if(!str.second) str.first += ";";
            result.second += "\n" + str.first;
        }
        return result;
    }

    virtual void GenerateFunctionPrototype(const std::string& name, std::vector<Value> args, Type ret, bool mangle = true, bool hasScope = false) = 0;
    virtual void GenerateMemberFunctionPrototype(const std::string& name, std::vector<Value> args, Type ret, bool hasScope = false) = 0;
    virtual void GenerateStruct(const std::string& name) = 0;
    virtual void GenerateEnum(const std::string& name) = 0;
    virtual void GenerateStructScopeStart() = 0;
    virtual void GenerateStructScopeEnd() = 0;
    virtual void GenerateScopeStart(bool inScope = false) = 0;
    virtual void GenerateScopeEnd(bool inScope = false) = 0;

    virtual void GenerateInclude(const std::string& filename) = 0;

    virtual void GenerateVariableDeclaration(const Value& val) = 0;
    virtual void GenerateVariableDeclarationWithAssignment(const Value& val) = 0;
    virtual void GenerateMemberVariableDeclaration(const Value& val) = 0;

    virtual void GenerateScopedVariableDeclaration(const Value& val) = 0;
    virtual void GenerateScopedVariableAssignment(const std::string& val, const std::string& assignment = "=") = 0;
    virtual void GenerateScopedVariableDeclarationWithAssignment(const Value& val) = 0;
    virtual void BeginFunctionCall(const std::string& func) = 0;
    virtual void GenerateFunctionCall(const std::vector<Type>& values, std::vector<std::pair<std::string, bool>> namespaces) = 0;
    virtual void GenerateIf() = 0;
    virtual void GenerateElse() = 0;
    virtual void GenerateElseIf() = 0;
    virtual void GenerateWhile() = 0;
    virtual void GenerateBreak() = 0;
    virtual void GenerateContinue() = 0;
    virtual void GenerateReturn() = 0;
    virtual void GenerateString(const std::string& str, bool inScope = false) = 0;
    virtual void GenerateLineEnd(bool inScope = false) = 0;

    virtual void PushNamespaceStack(const std::string& name, bool isStruct = false) = 0;
    virtual void PopNamespaceStack() = 0;

    virtual std::string Mangle(const std::string& name, const std::vector<Type>& types = {}) = 0;
    virtual std::string Mangle(const std::string& name, const std::vector<Value>& types) = 0;

    Type ConstructType(const std::string& type, const std::string& modifiers)
    {
        Type result;

        size_t modPos = 0;
        Type* curr = &result;
        while(true)
        {
            if(modifiers[modPos] == '[')
            {
                curr->id = Type::ArrayID;
                curr->next = new Type();
                curr = curr->next;
                modPos += 2;
            }
            if(modifiers[modPos] == '*')
            {
                curr->id = Type::PointerID;
                curr->next = new Type();
                curr = curr->next;
                modPos++;
            }
            else break;
        }
        curr->id = typeIDs[type];
        curr->structName = type;
        return result;
    }

    std::string DeconstructType(Type type)
    {
        std::string result = "";
        std::string modifiers = "";

        Type* curr = &type;
        while(curr->next != nullptr)
        {
            modifiers += typeNameMap[curr->id];
            curr = curr->next;
        }

        if(curr->id == Type::StructID || curr->id == Type::ERRID) result += curr->structName;
        else result += typeNameMap[curr->id];

        return result + modifiers;
    }

    std::string MangleType(Type type)
    {
        std::string result = "";
        std::string modifiers = "";
        std::string name;

        Type* curr = &type;
        while(curr->next != nullptr)
        {
            modifiers += typeMangleMap[curr->id];
            curr = curr->next;
        }

        if(curr->id == Type::StructID || curr->id == Type::ERRID)
        {
            name = curr->structName;
            result += std::to_string(name.length());
        }
        else name = typeMangleMap[curr->id];

        result += name;
        result += modifiers;

        return result;
    }

    std::string GenerateMangledNamespace(const std::vector<std::pair<std::string, bool>>& namespaces)
    {
        if(namespaces.empty()) return "";
        auto namespaceStack__ = namespaceStack;
        namespaceStack.clear();
        for(size_t i = 0; i < namespaces.size() - 1; i++)
        {
            namespaceStack.push_back(namespaces[i]);
            // undo any mangling on the current namespace
            auto& back = namespaceStack.back().first;
            if(back.front() == '_' && isDigit(back[1]))
            {
                back = back.substr(1);
                while(isDigit(back.front()))
                {
                    back = back.substr(1);
                }
            }
        }
        std::string result = Mangle(namespaces.back().first);
        namespaceStack = namespaceStack__;
        return result;
    }

    std::string GenerateMangledFunction(const std::vector<std::pair<std::string, bool>>& namespaces, const std::string& function, const std::vector<Type>& values, bool isVar = false, bool isMember = false)
    {
        if(isVar && namespaces.front().first == "this")
        {
            return "this->" + function;
        }
        else if(isVar && namespaces.empty())
        {
            return function;
        }
        else if(namespaces.empty() && primitive_set.count(function))
        {
            return function;
        }
        else
        {
            auto namespaceStack__ = namespaceStack;
            namespaceStack.clear();
            for(size_t i = 0; i < namespaces.size(); i++)
            {
                namespaceStack.push_back(namespaces[i]);
                // undo any mangling on the current namespace
                auto& back = namespaceStack.back().first;
                if(back.front() == '_' && isDigit(back[1]))
                {
                    back = back.substr(1);
                    while(isDigit(back.front()))
                    {
                        back = back.substr(1);
                    }
                }
            }
            std::string result = Mangle(function, values);
            namespaceStack = namespaceStack__;
            return result;
        }
    }

    bool IsMemberNamespace(const std::vector<std::pair<std::string, bool>>& namespaces)
    {
        for(auto& ns : namespaces)
        {
            if(ns.second) return true;
        }
        return false;
    }

    Type GetReturnType(const std::string& function)
    {
        return knownFunctions[function];
    }

    Type GetType(const std::string& var)
    {
        return knownVariables[var];
    }

    Type Promote(const Type& a, const Type& b)
    {
        size_t index_A, index_B;
        for(size_t i = 0; i < typePromoteOrder.size(); i++)
        {
            if(a.id == typePromoteOrder[i])
            {
                index_A = i;
            }
            if(b.id == typePromoteOrder[i])
            {
                index_B = i;
            }
        }
        if(index_A > index_B)
        {
            return a;
        }
        return b;
    }

    Type Dereference(const Type& type)
    {
        Type result = type;
        if(result.next == nullptr)
        {   // cannot dereference
            return type;
        }
        auto tmp = *result.next;
        result = tmp;
        return result;
    }

    void SetPublic(bool public_)
    {
        publicEnabled = public_;
    }

    void PushPublicDefaultStack(bool public_)
    {
        publicDefaultStack.push_back(public_);
    }

    void PopPublicDefaultStack()
    {
        publicDefaultStack.pop_back();
    }

    bool GetPublicDefault()
    {
        return publicDefaultStack.back();
    }

    bool GetPublic()
    {
        return publicEnabled;
    }

protected:
    std::vector<std::pair<std::string, bool>> output;
    std::vector<std::pair<std::string, bool>> headerOutput;
    std::vector<std::pair<std::string, bool>> namespaceStack;
    std::unordered_set<std::string> knownNamespaces;
    std::unordered_map<std::string, Type> knownFunctions;
    std::unordered_map<std::string, Type> knownVariables;
    bool publicEnabled = false;
    std::vector<bool> publicDefaultStack = {false};

    std::vector<Type::TypeID> typePromoteOrder =
    {
        Type::VoidID,
        Type::I1ID,
        Type::I8UID,
        Type::I8SID,
        Type::I16UID,
        Type::I16SID,
        Type::I32UID,
        Type::I32SID,
        Type::I64UID,
        Type::I64SID,
        Type::I128UID,
        Type::I128SID,
        Type::FP32ID,
        Type::FP64ID,
        Type::FP128ID,
        Type::ArrayID,
        Type::PointerID,
    };

    std::unordered_map<std::string, Type::TypeID> typeIDs =
    {
        {"0", Type::UNDEFID},
        {"", Type::ERRID},
        {"f32_t", Type::FP32ID},
        {"f64_t", Type::FP64ID},
        {"f128_t", Type::FP128ID},
        {"void", Type::VoidID},
        {"i8_t", Type::I8SID},
        {"i16_t", Type::I16SID},
        {"i32_t", Type::I32SID},
        {"i64_t", Type::I64SID},
        {"i128_t", Type::I128SID},
        {"u8_t", Type::I8UID},
        {"u16_t", Type::I16UID},
        {"u32_t", Type::I32UID},
        {"u64_t", Type::I64UID},
        {"u128_t", Type::I128UID},
        {"bool", Type::I1ID},
        {"char", Type::I8UID},
        {"byte", Type::I8UID},
    };

    std::unordered_map<Type::TypeID, std::string> typeNameMap =
    {
        {Type::UNDEFID, "__TYPE_UNDEFINED__"},
        {Type::ERRID, "__TYPE_ERROR__"},
        {Type::FP32ID, "float"},
        {Type::FP64ID, "double"},
        {Type::FP128ID, "long double"},
        {Type::VoidID, "void"},
        {Type::I8SID, "signed char"},
        {Type::I16SID, "signed short"},
        {Type::I32SID, "signed int"},
        {Type::I64SID, "signed long long"},
        {Type::I128SID, "__int128_t"},
        {Type::I8UID, "unsigned char"},
        {Type::I16UID, "unsigned short"},
        {Type::I32UID, "unsigned int"},
        {Type::I64UID, "unsigned long long"},
        {Type::I128UID, "__uint128_t"},
        {Type::I1ID, "bool"},
        {Type::I8UID, "unsigned char"},
        {Type::I8UID, "unsigned char"},
        {Type::ArrayID, "[]"},
        {Type::PointerID, "*"},
    };

    std::unordered_map<Type::TypeID, std::string> typeMangleMap =
    {
        {Type::UNDEFID, "_UNDEFINED_"},
        {Type::ERRID, "_ERROR_"},
        {Type::FP32ID, "n"},
        {Type::FP64ID, "n"},
        {Type::FP128ID, "n"},
        {Type::VoidID, "v"},
        {Type::I8SID, "n"},
        {Type::I16SID, "n"},
        {Type::I32SID, "n"},
        {Type::I64SID, "n"},
        {Type::I128SID, "n"},
        {Type::I8UID, "n"},
        {Type::I16UID, "n"},
        {Type::I32UID, "n"},
        {Type::I64UID, "n"},
        {Type::I128UID, "n"},
        {Type::I1ID, "b"},
        {Type::I8UID, "n"},
        {Type::I8UID, "n"},
        {Type::ArrayID, "a"},
        {Type::PointerID, "p"},
    };

    std::unordered_set<std::string> primitive_set =
    {
        {"f32_t"},
        {"f64_t"},
        {"f128_t"},
        {"void"},
        {"i8_t"},
        {"i16_t"},
        {"i32_t"},
        {"i64_t"},
        {"i128_t"},
        {"u8_t"},
        {"u16_t"},
        {"u32_t"},
        {"u64_t"},
        {"u128_t"},
        {"bool"},
        {"char"},
        {"byte"},
    };
};