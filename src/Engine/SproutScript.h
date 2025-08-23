#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

/**
 * Sprout Script (.sp) - A simplified, fun C++ wrapper language
 *
 * Example Sprout Script:
 * ```
 * actor MyActor extends Actor {
 *     var health: float = 100.0
 *     var speed: float = 5.0
 *
 *     fun beginPlay() {
 *         print("Hello from MyActor!")
 *         setLocation(0, 0, 0)
 *     }
 *
 *     fun tick(deltaTime: float) {
 *         moveForward(speed * deltaTime)
 *         if (health <= 0) {
 *             destroy()
 *         }
 *     }
 *
 *     fun takeDamage(amount: float) {
 *         health -= amount
 *         print("Ouch! Health now: " + health)
 *     }
 * }
 * ```
 */

// Token types for lexical analysis
enum class TokenType {
    // Literals
    IDENTIFIER,
    NUMBER,
    STRING,

    // Keywords
    ACTOR,
    EXTENDS,
    VAR,
    FUN,
    IF,
    ELSE,
    WHILE,
    FOR,
    RETURN,
    TRUE,
    FALSE,

    // Types
    INT,
    FLOAT,
    STRING_TYPE,
    BOOL,
    VECTOR3,

    // Operators
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    ASSIGN,
    EQUALS,
    NOT_EQUALS,
    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,

    // Delimiters
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    COLON,
    SEMICOLON,

    // Special
    NEWLINE,
    END_OF_FILE
};

// Token structure
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType t, const std::string& v, int l, int c)
        : type(t), value(v), line(l), column(c) {}
};

// AST Node types
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string GenerateCpp() const = 0;
};

class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// Literal expressions
class NumberLiteral : public Expression {
public:
    float value;
    NumberLiteral(float v) : value(v) {}
    std::string GenerateCpp() const override {
        return std::to_string(value);
    }
};

class StringLiteral : public Expression {
public:
    std::string value;
    StringLiteral(const std::string& v) : value(v) {}
    std::string GenerateCpp() const override {
        return "\"" + value + "\"";
    }
};

class BoolLiteral : public Expression {
public:
    bool value;
    BoolLiteral(bool v) : value(v) {}
    std::string GenerateCpp() const override {
        return value ? "true" : "false";
    }
};

class Identifier : public Expression {
public:
    std::string name;
    Identifier(const std::string& n) : name(n) {}
    std::string GenerateCpp() const override {
        return name;
    }
};

// Binary operations
class BinaryOperation : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::string operator_;
    std::unique_ptr<Expression> right;

    BinaryOperation(std::unique_ptr<Expression> l, const std::string& op, std::unique_ptr<Expression> r)
        : left(std::move(l)), operator_(op), right(std::move(r)) {}

    std::string GenerateCpp() const override {
        return "(" + left->GenerateCpp() + " " + operator_ + " " + right->GenerateCpp() + ")";
    }
};

// Function call
class FunctionCall : public Expression {
public:
    std::string functionName;
    std::vector<std::unique_ptr<Expression>> arguments;

    FunctionCall(const std::string& name) : functionName(name) {}

    std::string GenerateCpp() const override {
        std::string result = TranslateFunctionName(functionName) + "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) result += ", ";
            result += arguments[i]->GenerateCpp();
        }
        result += ")";
        return result;
    }

private:
    std::string TranslateFunctionName(const std::string& name) const {
        // Translate Sprout Script function names to C++ equivalents
        static std::unordered_map<std::string, std::string> translations = {
            {"print", "std::cout << "},
            {"setLocation", "SetActorLocation"},
            {"getLocation", "GetActorLocation"},
            {"setRotation", "SetActorRotation"},
            {"getRotation", "GetActorRotation"},
            {"moveForward", "AddActorWorldOffset"},
            {"destroy", "MarkForDestroy"},
        };

        auto it = translations.find(name);
        return (it != translations.end()) ? it->second : name;
    }
};

// Variable declaration
class VariableDeclaration : public Statement {
public:
    std::string name;
    std::string type;
    std::unique_ptr<Expression> initialValue;

    VariableDeclaration(const std::string& n, const std::string& t)
        : name(n), type(t) {}

    std::string GenerateCpp() const override {
        std::string result = TranslateType(type) + " " + name;
        if (initialValue) {
            result += " = " + initialValue->GenerateCpp();
        }
        result += ";";
        return result;
    }

private:
    std::string TranslateType(const std::string& type) const {
        static std::unordered_map<std::string, std::string> typeMap = {
            {"int", "int"},
            {"float", "float"},
            {"string", "std::string"},
            {"bool", "bool"},
            {"vector3", "glm::vec3"},
        };

        auto it = typeMap.find(type);
        return (it != typeMap.end()) ? it->second : type;
    }
};

// Assignment statement
class Assignment : public Statement {
public:
    std::string variableName;
    std::unique_ptr<Expression> value;

    Assignment(const std::string& name, std::unique_ptr<Expression> val)
        : variableName(name), value(std::move(val)) {}

    std::string GenerateCpp() const override {
        return variableName + " = " + value->GenerateCpp() + ";";
    }
};

// If statement
class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> thenBody;
    std::vector<std::unique_ptr<Statement>> elseBody;

    IfStatement(std::unique_ptr<Expression> cond) : condition(std::move(cond)) {}

    std::string GenerateCpp() const override {
        std::string result = "if (" + condition->GenerateCpp() + ") {\n";
        for (const auto& stmt : thenBody) {
            result += "    " + stmt->GenerateCpp() + "\n";
        }
        result += "}";

        if (!elseBody.empty()) {
            result += " else {\n";
            for (const auto& stmt : elseBody) {
                result += "    " + stmt->GenerateCpp() + "\n";
            }
            result += "}";
        }

        return result;
    }
};

// Function definition
class FunctionDefinition : public ASTNode {
public:
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters; // name, type
    std::string returnType;
    std::vector<std::unique_ptr<Statement>> body;

    FunctionDefinition(const std::string& n) : name(n), returnType("void") {}

    std::string GenerateCpp() const override {
        std::string result = TranslateFunctionName(name) + "(";

        // Add parameters
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) result += ", ";
            result += TranslateType(parameters[i].second) + " " + parameters[i].first;
        }
        result += ") {\n";

        // Add body
        for (const auto& stmt : body) {
            result += "    " + stmt->GenerateCpp() + "\n";
        }

        result += "}\n";
        return result;
    }

private:
    std::string TranslateFunctionName(const std::string& name) const {
        if (name == "beginPlay") return "void BeginPlay() override";
        if (name == "tick") return "void Tick(float deltaTime) override";
        if (name == "endPlay") return "void EndPlay() override";
        return "void " + name;
    }

    std::string TranslateType(const std::string& type) const {
        static std::unordered_map<std::string, std::string> typeMap = {
            {"int", "int"},
            {"float", "float"},
            {"string", "std::string"},
            {"bool", "bool"},
            {"vector3", "glm::vec3"},
        };

        auto it = typeMap.find(type);
        return (it != typeMap.end()) ? it->second : type;
    }
};

// Actor class definition
class ActorClassDefinition : public ASTNode {
public:
    std::string className;
    std::string baseClass;
    std::vector<std::unique_ptr<VariableDeclaration>> variables;
    std::vector<std::unique_ptr<FunctionDefinition>> functions;

    ActorClassDefinition(const std::string& name, const std::string& base = "Actor")
        : className(name), baseClass(base) {}

    std::string GenerateCpp() const override {
        std::string result = "class " + className + " : public " + baseClass + " {\n";
        result += "public:\n";

        // Constructor
        result += "    " + className + "(World* world, const std::string& name = \"" + className + "\")\n";
        result += "        : " + baseClass + "(world, name) {\n";
        result += "    }\n\n";

        // Member functions
        for (const auto& func : functions) {
            std::string funcCode = func->GenerateCpp();
            // Indent the function
            size_t pos = 0;
            while ((pos = funcCode.find('\n', pos)) != std::string::npos) {
                funcCode.insert(pos + 1, "    ");
                pos += 5;
            }
            result += "    " + funcCode + "\n";
        }

        // Private members
        result += "private:\n";
        for (const auto& var : variables) {
            result += "    " + var->GenerateCpp() + "\n";
        }

        result += "};\n";
        return result;
    }
};

// Lexer class
class SproutLexer {
public:
    SproutLexer(const std::string& source);
    std::vector<Token> Tokenize();

private:
    std::string source;
    size_t current;
    int line;
    int column;

    char CurrentChar() const;
    char NextChar();
    void SkipWhitespace();
    Token ReadNumber();
    Token ReadString();
    Token ReadIdentifier();
    bool IsAlpha(char c) const;
    bool IsDigit(char c) const;
    bool IsAlphaNumeric(char c) const;
};

// Parser class
class SproutParser {
public:
    SproutParser(const std::vector<Token>& tokens);
    std::unique_ptr<ActorClassDefinition> Parse();

private:
    std::vector<Token> tokens;
    size_t current;

    Token CurrentToken() const;
    Token NextToken();
    bool Match(TokenType type);
    bool Check(TokenType type) const;

    std::unique_ptr<ActorClassDefinition> ParseActorClass();
    std::unique_ptr<VariableDeclaration> ParseVariableDeclaration();
    std::unique_ptr<FunctionDefinition> ParseFunctionDefinition();
    std::unique_ptr<Statement> ParseStatement();
    std::unique_ptr<Expression> ParseExpression();
    std::unique_ptr<Expression> ParsePrimary();
};

// Code generator
class SproutCodeGenerator {
public:
    static std::string GenerateCppFile(const ActorClassDefinition& actorClass);
    static std::string GenerateHeaderFile(const ActorClassDefinition& actorClass);
    static bool CompileToSharedLibrary(const std::string& cppCode, const std::string& outputPath);
};

// Main compiler interface
class SproutCompiler {
public:
    static bool CompileFile(const std::string& sproutFilePath, const std::string& outputDir);
    static bool CompileString(const std::string& sproutCode, const std::string& outputPath);
};
