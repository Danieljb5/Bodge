#include "tokens.hpp"
#include "backend.hpp"
#include <string>

Token lex();
std::string get_id();
void error(const std::string& msg);
size_t getLine();

Token tok;
BackendGenerator* generator;

void next()
{
    tok = lex();
    // std::cout << "[" << TokenNames[tok] << "]";
}

bool accept(Token t)
{
    if(tok == t)
    {
        next();
        return true;
    }
    return false;
}

bool expect(Token t, const std::string& in = "")
{
    if(accept(t)) return true;

    std::string msg = "unexpected " + TokenNames[tok] + ", expected " + TokenNames[t];
    if(in != "") msg += " in " + in;
    error(msg);
    return false;
}

Type arithexpr(bool inScope = false);
Type relationalexpr(bool inScope = false);

void namespaces(std::vector<std::pair<std::string, bool>>& ns)
{
    if(accept(IDENTIFIER))
    {
        ns.push_back({get_id(), false});
    }
    if(accept(COLON)) namespaces(ns);
    if(accept(DOT))
    {
        ns.back().second = true;
        namespaces(ns);
    }
}

std::vector<std::pair<std::string, bool>> namespaces()
{
    std::vector<std::pair<std::string, bool>> result;
    namespaces(result);
    return result;
}

std::vector<std::pair<std::string, bool>> namespaces(std::string initial)
{
    std::vector<std::pair<std::string, bool>> result;
    result.push_back({initial, false});
    if(accept(COLON))
    {
        namespaces(result);
    }
    else if(accept(DOT))
    {
        result[0].second = true;
        namespaces(result);
    }
    return result;
}

std::vector<Type> callArgs();

void callArgs(std::vector<Type>& values)
{
    if(tok == CLOSE) return;
    if(accept(STRING))
    {
        Type t = Type::PointerID;
        t.next = new Type(Type::FP32ID);
        values.push_back(t);
        generator->GenerateString(get_id(), true);
    }
    else
    {
        values.push_back(relationalexpr(true));
    }
    
    if(accept(COMMA))
    {
        generator->GenerateString(", ", true);
        callArgs(values);
    }
}

std::vector<Type> callArgs()
{
    std::vector<Type> result;
    callArgs(result);
    return result;
}

Type value(bool inScope = false)
{
    Type type_(Type::VoidID);
    size_t ref_count = 0;
    while(accept(AND))
    {
        ref_count++;
    }
    size_t deref_count = 0;
    while(accept(MUL))
    {
        deref_count++;
    }
    if(accept(IDENTIFIER))
    {
        auto names = namespaces(get_id());
        auto name = names.back().first;
        names.pop_back();
        if(inScope && accept(OPEN))
        {   // function call
            generator->BeginFunctionCall(name);
            auto args = callArgs();
            generator->GenerateFunctionCall(args, names);
            auto mangledName = generator->GenerateMangledFunction(names, name, args);
            type_ = generator->GetReturnType(mangledName);
            expect(CLOSE, "function call");
        }
        else
        {
            auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
            generator->GenerateString(name_, inScope);
            auto tpe = generator->GetType(name_);
            for(size_t i = 0; i < ref_count; i++) tpe = generator->Reference(tpe);
            for(size_t i = 0; i < deref_count; i++) tpe = generator->Dereference(tpe);
            type_ = generator->Promote(type_, tpe);
        }
    }
    else if(accept(NUMBER))
    {
        type_ = generator->Promote(type_, Type::FP32ID);
        generator->GenerateString(get_id(), inScope);
    }
    else if(accept(OPEN))
    {
        generator->GenerateString("(", inScope);
        type_ = generator->Promote(type_, arithexpr(inScope));
        expect(CLOSE, "arithmetic expression");
        generator->GenerateString(")", inScope);
    }
    else
    {
        error("syntax error: expecting lvalue, rvalue or nested expression");
    }
    return type_;
}

Type addsub(bool inScope = false)
{
    if(tok == ADD || tok == SUB)
    {
        if(tok == ADD) generator->GenerateString("+", inScope);
        if(tok == SUB) generator->GenerateString("-", inScope);
        next();
    }
    auto type_ = value(inScope);
    while(tok == ADD || tok == SUB)
    {
        if(tok == ADD) generator->GenerateString("+", inScope);
        if(tok == SUB) generator->GenerateString("-", inScope);
        next();
        type_ = generator->Promote(type_, value(inScope));
    }
    return type_;
}

Type muldiv(bool inScope = false)
{
    bool deref = false;
    if(tok == MUL)
    {
        deref = true;
        generator->GenerateString("*", inScope);
        next();
    }
    auto type_ = addsub(inScope);
    if(deref) type_ = generator->Dereference(type_);
    while(tok == MUL || tok == DIV || tok == MOD)
    {
        if(tok == MUL) generator->GenerateString("*", inScope);
        if(tok == DIV) generator->GenerateString("/", inScope);
        if(tok == MOD) generator->GenerateString("%", inScope);
        next();
        type_ = generator->Promote(type_, addsub(inScope));
    }
    return type_;
}

Type arithexpr(bool inScope)
{
    bool ref = false;
    if(tok == AND)
    {
        ref = true;
        generator->GenerateString("&", inScope);
        next();
    }
    auto type_ = muldiv(inScope);
    if(ref) type_ = generator->Reference(type_);
    while(tok == AND || tok == OR || tok == XOR)
    {
        if(tok == AND) generator->GenerateString("&", inScope);
        if(tok == OR)  generator->GenerateString("|", inScope);
        if(tok == XOR) generator->GenerateString("^", inScope);
        next();
        type_ = generator->Promote(type_, muldiv(inScope));
    }
    return type_;
}

Type relationalexpr(bool inScope)
{
    auto type_ = arithexpr(inScope);
    Type type = Type::I1ID;
    if(accept(GT))
    {
        generator->GenerateString(">", inScope);
        arithexpr(inScope);
    }
    else if(accept(GE))
    {
        generator->GenerateString(">=", inScope);
        arithexpr(inScope);
    }
    else if(accept(LT))
    {
        generator->GenerateString("<", inScope);
        arithexpr(inScope);
    }
    else if(accept(LE))
    {
        generator->GenerateString("<=", inScope);
        arithexpr(inScope);
    }
    else if(accept(EQ))
    {
        generator->GenerateString("==", inScope);
        arithexpr(inScope);
    }
    else if(accept(NEQ))
    {
        generator->GenerateString("!=", inScope);
        arithexpr(inScope);
    }
    else
    {
        type = type_;
    }

    if(accept(LOGAND))
    {
        generator->GenerateString("&&", inScope);
        relationalexpr(inScope);
        type = Type::I1ID;
    }
    if(accept(LOGOR))
    {
        generator->GenerateString("||", inScope);
        relationalexpr(inScope);
        type = Type::I1ID;
    }

    return type;
}

std::string typemods()
{
    std::string result;
    if(accept(MUL))
    {
        result += "*";
    }
    else if(accept(OPENSQ))
    {
        expect(CLOSESQ, "type modifiers");
        result += "[]";
    }
    else
    {
        return result;
    }
    return result + typemods();
}

void args(std::vector<Value>& values)
{
    if(accept(IDENTIFIER))
    {
        auto names = namespaces(get_id());
        auto name = names.back().first;
        names.pop_back();
        auto type_mods = typemods();
        expect(IDENTIFIER, "function arguments");
        auto value_name = get_id();
        Value value;
        auto type_name = generator->GenerateMangledFunction(names, name, {});
        value.type = generator->ConstructType(type_name, type_mods);
        value.type.structName = type_name;
        value.name = value_name;
        values.push_back(value);
    }
    if(accept(COMMA)) args(values);
}

std::vector<Value> args()
{
    std::vector<Value> result;
    args(result);
    return result;
}

void scope()
{
    while(true)
    {
        size_t ref_count = 0;
        while(accept(AND))
        {
            ref_count++;
        }
        size_t deref_count = 0;
        while(accept(MUL))
        {
            deref_count++;
        }
        if(tok == IDENTIFIER)
        {
            auto names = namespaces();
            auto name = names.back().first;
            names.pop_back();
            if(accept(OPEN))
            {   // function call
                generator->BeginFunctionCall(name);
                auto args = callArgs();
                generator->GenerateFunctionCall(args, names);
                expect(CLOSE, "function call");
                expect(SEMICOLON, "function call");
                generator->GenerateLineEnd(true);
            }
            else if(accept(ASSIGN))
            {   // variable assignment
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_);
                if(accept(STRING))
                {
                    generator->GenerateString(get_id(), true);
                }
                else
                {
                    arithexpr(true);
                }
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(ADDEQUAL))
            {   // variable assignment
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "+=");
                if(accept(STRING))
                {
                    generator->GenerateString(get_id(), true);
                }
                else
                {
                    arithexpr(true);
                }
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(SUBEQUAL))
            {   // variable assignment
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "-=");
                if(accept(STRING))
                {
                    generator->GenerateString(get_id(), true);
                }
                else
                {
                    arithexpr(true);
                }
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(MULEQUAL))
            {   // variable assignment
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "*=");
                if(accept(STRING))
                {
                    generator->GenerateString(get_id(), true);
                }
                else
                {
                    arithexpr(true);
                }
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(DIVEQUAL))
            {   // variable assignment
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "/=");
                if(accept(STRING))
                {
                    generator->GenerateString(get_id(), true);
                }
                else
                {
                    arithexpr(true);
                }
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(ADDADD))
            {   // variable assignment
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "++");
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(SUBSUB))
            {   // variable assignment
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "--");
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(ANDEQUAL))
            {
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "&=");
                if(accept(STRING))
                {
                    generator->GenerateString(get_id(), true);
                }
                else
                {
                    arithexpr(true);
                }
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(OREQUAL))
            {
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "|=");
                if(accept(STRING))
                {
                    generator->GenerateString(get_id(), true);
                }
                else
                {
                    arithexpr(true);
                }
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(XOREQUAL))
            {
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "^=");
                if(accept(STRING))
                {
                    generator->GenerateString(get_id(), true);
                }
                else
                {
                    arithexpr(true);
                }
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else if(accept(MODEQUAL))
            {
                auto name_ = generator->GenerateMangledFunction(names, name, {}, true);
                for(size_t i = 0; i < ref_count; i++) name_ = "&" + name_;
                for(size_t i = 0; i < deref_count; i++) name_ = name_ = "*" + name;
                generator->GenerateScopedVariableAssignment(name_, "%=");
                if(accept(STRING))
                {
                    generator->GenerateString(get_id(), true);
                }
                else
                {
                    arithexpr(true);
                }
                expect(SEMICOLON, "variable assignment");
                generator->GenerateLineEnd(true);
            }
            else
            {   // variable declaration
                std::string mods = typemods();
                expect(IDENTIFIER, "variable declaration");
                Value val;
                auto name_ = generator->GenerateMangledFunction(names, name, {});
                val.type = generator->ConstructType(name_, mods);
                val.name = get_id();
                if(accept(ASSIGN))
                {
                    generator->GenerateScopedVariableDeclarationWithAssignment(val);
                    if(accept(STRING))
                    {
                        generator->GenerateString(get_id(), true);
                    }
                    else
                    {
                        arithexpr(true);
                    }
                    expect(SEMICOLON, "variable assignment");
                    generator->GenerateLineEnd(true);
                }
                else
                {
                    generator->GenerateScopedVariableDeclaration(val);
                    expect(SEMICOLON, "variable declaration");
                }
            }
        }
        else if(accept(IF))
        {
            expect(OPEN, "if statement");
            generator->GenerateIf();
            relationalexpr(true);
            expect(CLOSE, "if statement");
            generator->GenerateString(")", true);
            expect(OPENB, "if body");
            generator->GenerateScopeStart(true);
            scope();
            generator->GenerateScopeEnd(true);
            expect(CLOSEB, "if body");
            bool hasElse = false;
            while(accept(ELSE) && !hasElse)
            {
                if(accept(IF))
                {
                    expect(OPEN, "else if statement");
                    generator->GenerateElseIf();
                    relationalexpr(true);
                    expect(CLOSE, "else if statement");
                    generator->GenerateString(")", true);
                    expect(OPENB, "else if body");
                    generator->GenerateScopeStart(true);
                    scope();
                    generator->GenerateScopeEnd(true);
                    expect(CLOSEB, "else if body");
                }
                else
                {
                    hasElse = true;
                    generator->GenerateElse();
                    expect(OPENB, "else body");
                    generator->GenerateScopeStart(true);
                    scope();
                    generator->GenerateScopeEnd(true);
                    expect(CLOSEB, "else body");
                }
            }
        }
        else if(accept(WHILE))
        {
            expect(OPEN, "while statement");
            generator->GenerateWhile();
            relationalexpr(true);
            expect(CLOSE, "while statement");
            generator->GenerateString(")", true);
            expect(OPENB, "while body");
            generator->GenerateScopeStart(true);
            scope();
            generator->GenerateScopeEnd(true);
            expect(CLOSEB, "while body");
        }
        else if(accept(BREAK))
        {
            generator->GenerateBreak();
            expect(SEMICOLON, "break");
        }
        else if(accept(CONTINUE))
        {
            generator->GenerateContinue();
            expect(SEMICOLON, "continue");
        }
        else if(accept(RETURN))
        {
            generator->GenerateReturn();
            if(accept(STRING))
            {
                generator->GenerateString(get_id(), true);
            }
            else
            {
                arithexpr(true);
            }
            expect(SEMICOLON, "return");
            generator->GenerateLineEnd(true);
        }
        else if(tok == CLOSEB) return;
        else
        {
            error("unexpected " + TokenNames[tok] + " in scope");
            return;
        }
    }
}

void enum_(const std::string& enumName)
{
    bool continue_ = true;
    while(continue_)
    {
        if(accept(IDENTIFIER))
        {
            generator->GenerateString(enumName + generator->Mangle(get_id()) + ",");
            if(!accept(COMMA))
            {
                continue_ = false;
            }
        }
        else if(tok == CLOSEB) return;
        else
        {
            error("Unexpected " + TokenNames[tok] + " in enum");
            return;
        }
    }
}

void group()
{
    while(true)
    {
        if(accept(IDENTIFIER))
        {
            auto names = namespaces(get_id());
            auto name = names.back().first;
            names.pop_back();
            if(accept(OPEN))
            {   // function prototype
                std::vector<Value> args_ = args();
                expect(CLOSE, "function prototype");
                std::string retType_ = "void";
                std::string retMods = "";
                if(accept(COLON))
                {   // function return type
                    expect(IDENTIFIER, "function return type");
                    auto names = namespaces(get_id());
                    if(names.size() > 1) retType_ = generator->GenerateMangledNamespace(names);
                    else retType_ = names.back().first;
                    retMods = typemods();
                }

                Type retType = generator->ConstructType(retType_, retMods);
                bool hasScope = (tok == OPENB);
                generator->GenerateMemberFunctionPrototype(name, args_, retType, hasScope);

                if(accept(OPENB))
                {   // function body
                    generator->GenerateScopeStart(true);
                    scope();
                    expect(CLOSEB, "function body");
                    generator->GenerateScopeEnd(true);
                }
                else
                {
                    expect(SEMICOLON, "function prototype");
                }
            }
            else if(accept(IDENTIFIER))
            {   // variable declaration
                Value val;
                auto name_ = generator->GenerateMangledFunction(names, name, {});
                val.type = generator->ConstructType(name_, "");
                val.name = get_id();
                generator->GenerateMemberVariableDeclaration(val);
                expect(SEMICOLON, "variable declaration");
            }
            else
            {
                std::string mods = typemods();
                if(accept(IDENTIFIER))
                {   // variable declaration
                    Value val;
                    auto name_ = generator->GenerateMangledFunction(names, name, {});
                    val.type = generator->ConstructType(name_, mods);
                    val.name = get_id();
                    generator->GenerateMemberVariableDeclaration(val);
                    expect(SEMICOLON, "variable declaration");
                }
                else
                {
                    error("syntax error, unexpected " + TokenNames[tok] + " in group");
                    return;
                }
            }
        }
        else if(tok == CLOSEB) return;
        else
        {
            error("Unexpected " + TokenNames[tok] + " in group");
            return;
        }
    }
}

void using_()
{
    if(accept(STRING))
    {
        generator->GenerateInclude(get_id());
    }
    else
    {
        auto path = namespaces();
        std::string filename = "\"";
        for(size_t i = 0; i < path.size(); i++)
        {
            filename += path[i].first;
            if(i + 1 < path.size()) filename += "/";
        }
        filename += ".h\"";
        generator->GenerateInclude(filename);
    }
}

void block()
{
    while(true)
    {
        if(accept(NAMESPACE))
        {
            expect(IDENTIFIER, "namespace declaration");
            std::string name = get_id();
            generator->PushNamespaceStack(name);
            generator->PushPublicDefaultStack(generator->GetPublic());
            expect(OPENB, "namespace declaration");
            block();
            expect(CLOSEB, "namespace declaration");
            generator->PopPublicDefaultStack();
            generator->PopNamespaceStack();
            generator->SetPublic(generator->GetPublicDefault());
        }
        else if(accept(GROUP))
        {
            expect(IDENTIFIER, "group declaration");
            std::string name = get_id();
            generator->GenerateStruct(name);
            generator->GenerateStructScopeStart();
            generator->PushNamespaceStack(name, true);
            expect(OPENB, "group declaration");
            group();
            expect(CLOSEB, "group declaration");
            generator->PopNamespaceStack();
            generator->GenerateStructScopeEnd();
            generator->SetPublic(generator->GetPublicDefault());
        }
        else if(accept(ENUM))
        {
            expect(IDENTIFIER, "enum declaration");
            std::string name = generator->Mangle(get_id());
            generator->GenerateEnum(name);
            expect(OPENB, "enum body");
            generator->GenerateScopeStart();
            enum_(name);
            expect(CLOSEB, "enum body");
            generator->GenerateScopeEnd();
            generator->GenerateLineEnd();
            generator->SetPublic(generator->GetPublicDefault());
        }
        else if(accept(PUBLIC))
        {
            generator->SetPublic(true);
        }
        else if(accept(PRIVATE))
        {
            generator->SetPublic(false);
        }
        else if(accept(USING))
        {
            using_();
            generator->SetPublic(generator->GetPublicDefault());
        }
        else if(accept(IDENTIFIER))
        {
            auto names = namespaces(get_id());
            auto name = names.back().first;
            names.pop_back();
            if(accept(OPEN))
            {   // function prototype
                std::vector<Value> args_ = args();
                expect(CLOSE, "function prototype");
                std::string retType_ = "void";
                std::string retMods = "";
                if(accept(COLON))
                {   // function return type
                    expect(IDENTIFIER, "function return type");
                    auto names = namespaces(get_id());
                    if(names.size() > 1) retType_ = generator->GenerateMangledNamespace(names);
                    else retType_ = names.back().first;
                    retMods = typemods();
                }

                Type retType = generator->ConstructType(retType_, retMods);
                bool mangle = (name != "main");
                bool hasScope = (tok == OPENB);
                generator->GenerateFunctionPrototype(name, args_, retType, mangle, hasScope);

                if(accept(OPENB))
                {   // function body
                    generator->GenerateScopeStart(true);
                    scope();
                    expect(CLOSEB, "function body");
                    generator->GenerateScopeEnd(true);
                }
                else
                {
                    expect(SEMICOLON, "function prototype");
                }
                generator->SetPublic(generator->GetPublicDefault());
            }
            else if(accept(IDENTIFIER))
            {   // variable declaration
                Value val;
                val.type = generator->ConstructType(name, "");
                val.name = get_id();
                if(accept(ASSIGN))
                {
                    generator->GenerateVariableDeclarationWithAssignment(val);
                    if(accept(STRING))
                    {
                        generator->GenerateString(get_id());
                    }
                    else
                    {
                        arithexpr();
                    }
                }
                else
                {
                    generator->GenerateVariableDeclaration(val);
                }
                expect(SEMICOLON, "variable declaration");
                generator->SetPublic(generator->GetPublicDefault());
            }
            else
            {
                std::string mods = typemods();
                if(accept(IDENTIFIER))
                {   // variable declaration
                    Value val;
                    val.type = generator->ConstructType(name, mods);
                    val.name = get_id();
                    if(accept(ASSIGN))
                    {
                        generator->GenerateVariableDeclarationWithAssignment(val);
                        if(accept(STRING))
                        {
                            generator->GenerateString(get_id());
                        }
                        else
                        {
                            arithexpr();
                        }
                    }
                    else
                    {
                        generator->GenerateVariableDeclaration(val);
                    }
                    expect(SEMICOLON, "variable declaration");
                    generator->SetPublic(generator->GetPublicDefault());
                }
                else
                {
                    error("syntax error, unexpected " + TokenNames[tok]);
                    return;
                }
            }
        }
        else if(accept(T_EOF)) return;
        else if(tok == CLOSEB) return;
        else
        {
            error("syntax error, unexpected " + TokenNames[tok]);
            return;
        }
    }
}

void parse(BackendGenerator* _generator)
{
    generator = _generator;
    next();
    block();
}