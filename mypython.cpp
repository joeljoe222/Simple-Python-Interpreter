#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
//                                  TOKEN
//////////////////////////////////////////////////////////////////////////////////
enum TokenType
{
    IDENTIFIER,               // 0
    NUMBER,                   // 1
    PLUS,                     // 2
    MINUS,                    // 3
    MULTIPLY,                 // 4
    DIVIDE,                   // 5
    LPAREN,                   // 6
    RPAREN,                   // 7
    COMMA,                    // 8
    HASHTAG,                  // 9
    COLON,                    // 10
    GREATER_THAN,             // 11
    LESS_THAN,                // 12
    GREATER_THAN_OR_EQUAL_TO, // 13
    LESS_THAN_OR_EQUAL_TO,    // 14
    SINGLE_EQUAL,             // 15
    DOUBLE_EQUAL,             // 16
    STRING,                   // 17
    PRINT,                    // 18
    LEFT_BRACKET,             // 19
    RIGHT_BRACKET,            // 20
    IF,                       // 21
    ELSE,                     // 22
    DEF,                      // 23
    CALL_FUNC,                // 24
    LOCAL,                    // 25
    RETURN                    // 26
};

struct Token
{
    TokenType type;
    string value;
    // token will contain a token type and the value
    Token(TokenType type, const string &value) : type(type), value(value) {}
};
//////////////////////////////////////////////////////////////////////////////////
//                                  SYMBOL TABLE
//////////////////////////////////////////////////////////////////////////////////
// symbol table will store all the globalVars and its values to use later
class SymbolTable
{
private:
    unordered_map<string, int> globalVars;
    unordered_map<string, int> localVars;
    unordered_map<string, string> func_declaration;

public:
    // use to add variable name and its value into the dictionary (global)
    void addGlobalVar(const string &name, int value)
    {
        globalVars[name] = value;
    }
    void addLocalVar(const string &name, int value)
    {
        localVars[name] = value;
    }
    // will return to value of the coresponding variable name
    int getGlobalVar(const string &name)
    {
        int count = 0;
        if (globalVars.find(name) != globalVars.end())
        {
            int result = globalVars.at(name);
            if (func_declaration.find(name) != func_declaration.end())
            {
                count++;
            }
            if (func_declaration.find(name) != func_declaration.end() && count == 1)
            {
                globalVars.erase(name);
            }
            return result;
        }
        else
        {
            cerr << "Error: Variable " << name << " not found in global scope." << endl;
            exit(1);
        }
    }
    int getLocalVar(const string &name) const
    {
        if (localVars.find(name) != localVars.end())
        {
            return localVars.at(name);
        }
        else
        {
            cerr << "Error: Variable " << name << " not found in local scope." << endl;
            exit(1);
        }
    }

    void addFuncInit(const string &func_name, const vector<string> &parameters)
    {
        string parameter_list;
        for (const string &param : parameters)
        {
            parameter_list += param + ",";
        }
        if (!parameters.empty())
        {
            parameter_list.pop_back();
        }
        func_declaration[func_name] = parameter_list;
    }
    // get function parameter
    string getFunc(const string &func_name) const
    {
        if (func_declaration.find(func_name) != func_declaration.end())
        {
            return func_declaration.at(func_name);
        }
        else
        {
            cerr << "Error: Variable " << func_name << " not found." << endl;
            exit(1);
        }
    }

    bool isInFuncList(const string &func_name) const
    {
        if (func_declaration.find(func_name) != func_declaration.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    bool isInGlobalList(const string &func_name) const
    {
        if (globalVars.find(func_name) != globalVars.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    bool isInLocalList(const string &func_name) const
    {
        if (localVars.find(func_name) != localVars.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    void printLocalVars() const
    {
        cout << "Local Variables:" << endl;
        for (const auto &pair : localVars)
        {
            cout << pair.first << " : " << pair.second << endl;
        }
    }
    void printGlobalVars() const
    {
        cout << "Global Variables:" << endl;
        for (const auto &pair : globalVars)
        {
            cout << pair.first << " : " << pair.second << endl;
        }
    }
    void printFuncInit() const
    {
        cout << "Function Declarations:" << endl;
        for (const auto &pair : func_declaration)
        {
            cout << "Function: " << pair.first << "(" << pair.second << ")" << endl;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////
//                                  LEXER
//////////////////////////////////////////////////////////////////////////////////
class Lexer
{
private:
    string input;
    size_t position;
    vector<Token> tokens;
    char currentChar()
    {
        if (position < input.length())
            return input[position];
        return '\0';
    }

    void advance()
    {
        position++;
    }

    void skipWhitespace()
    {
        while (isspace(currentChar()))
            advance();
    }

    string readIdentifier()
    {
        string result;
        while (isalnum(currentChar()))
        {
            result += currentChar();
            advance();
        }
        return result;
    }

    string readNumber()
    {
        string result;
        while (isdigit(currentChar()))
        {
            result += currentChar();
            advance();
        }
        return result;
    }
    string makeString()
    {
        string result;
        // Skip the opening double quote
        advance();

        while (currentChar() != '"' && currentChar() != '\0')
        {
            result += currentChar();
            advance();
        }
        // Check if the string is terminated properly
        if (currentChar() == '\0')
        {
            cerr << "Error: Unterminated string literal." << endl;
            exit(1);
        }
        // Skip the closing double quote
        advance();

        return result;
    }
    Token makeEquals()
    {
        string result = "=";
        advance();
        // Check if the next character is an equal sign
        if (currentChar() == '=')
        {
            result += '=';
            advance();
            return Token(DOUBLE_EQUAL, result); // Return double equal if the next character is also an equal sign
        }
        else
        {
            return Token(SINGLE_EQUAL, result); // Return single equal otherwise
        }
    }

    Token makeGreaterThan()
    {
        string result = ">";
        advance();
        if (currentChar() == '=')
        {
            result += '=';
            advance();
            return Token(GREATER_THAN_OR_EQUAL_TO, result);
        }
        else
        {
            return Token(GREATER_THAN, result);
        }
    }

    Token makeLessThan()
    {
        string result = "<";
        advance();
        if (currentChar() == '=')
        {
            result += '=';
            advance();
            return Token(LESS_THAN_OR_EQUAL_TO, result);
        }
        else
        {
            return Token(LESS_THAN, result);
        }
    }

public:
    Lexer(string input) : input(input), position(0)
    {
    }

    const vector<Token> &getTokens() const
    {
        return tokens;
    }
    // create tokens
    void tokenize(SymbolTable &SymbolTable)
    {
        while (currentChar() != '\0')
        {
            // meaning it start with a space belong to either if or else
            if (isspace(currentChar()))
            {
                while (isspace(currentChar()))
                {
                    advance();
                }
                // tokenize the part after the spaces
                tokenize(SymbolTable);
            }
            else if (isalpha(currentChar())) // Check if it's an identifier or keyword
            {
                string identifier = readIdentifier();
                if (identifier == "if")
                {
                    tokens.push_back(Token(IF, "if"));
                    advance();
                    tokenize(SymbolTable);
                }
                else if (identifier == "else")
                {
                    tokens.push_back(Token(ELSE, "else"));
                    advance();
                    tokens.push_back(Token(COLON, ":"));
                }
                else if (identifier == "print")
                {
                    tokens.push_back(Token(PRINT, "print"));

                    // Check for '(' after 'print'
                    if (currentChar() != '(')
                    {
                        cerr << "Error: Expected '(' after 'print'" << endl;
                        exit(1);
                    }
                    tokens.push_back(Token(LPAREN, "(")); // Push left parenthesis token
                    advance();                            // Move past '('

                    // Parse and tokenize print arguments
                    while (currentChar() != ')')
                    {
                        // Skip whitespace between arguments
                        skipWhitespace();

                        // Parse and tokenize the argument
                        if (currentChar() == '"')
                        {
                            tokens.push_back(Token(STRING, makeString()));
                        }
                        else
                        {
                            tokens.push_back(Token(IDENTIFIER, readIdentifier()));
                        }

                        // Skip whitespace between arguments
                        skipWhitespace();

                        // Check for ',' to parse next argument
                        if (currentChar() == ',')
                            advance();
                        else if (currentChar() != ')' && currentChar() != '\0')
                        {
                            cerr << "Error: Expected ',' or ')' after argument" << endl;
                            exit(1);
                        }
                        else if (currentChar() == ')')
                        {
                            advance(); // Move past ')'
                            break;     // Exit the loop when encountering the end of the argument list
                        }
                    }
                    tokens.push_back(Token(RPAREN, ")")); // Push right parenthesis token
                }
                else if (identifier == "def")
                {
                    tokens.push_back(Token(DEF, "def"));
                }
                else if (SymbolTable.isInFuncList(identifier) && !SymbolTable.isInGlobalList(identifier))
                {
                    tokens.push_back(Token(CALL_FUNC, identifier));
                }
                else if (SymbolTable.isInGlobalList(identifier) && SymbolTable.isInFuncList(identifier))
                {
                    tokens.push_back(Token(IDENTIFIER, identifier));
                }
                else if (identifier == "return")
                {
                    tokens.push_back(Token(RETURN, "return"));
                }
                else if (identifier == "local")
                {
                    tokens.push_back(Token(LOCAL, identifier));
                }
                else
                {
                    tokens.push_back(Token(IDENTIFIER, identifier));
                }
            }
            else if (isdigit(currentChar()))
            {
                tokens.push_back(Token(NUMBER, readNumber()));
            }
            else if (currentChar() == '#')
            {
                // Skip over comment until end of line
                while (currentChar() != '\n' && currentChar() != '\0')
                    advance();
                continue;
            }
            else
            {
                switch (currentChar())
                {
                case '+':
                    tokens.push_back(Token(PLUS, "+"));
                    advance();
                    break;
                case '-':
                    tokens.push_back(Token(MINUS, "-"));
                    advance();
                    break;
                case '*':
                    tokens.push_back(Token(MULTIPLY, "*"));
                    advance();
                    break;
                case '/':
                    tokens.push_back(Token(DIVIDE, "/"));
                    advance();
                    break;
                case '(':
                    tokens.push_back(Token(LPAREN, "("));
                    advance();
                    break;
                case ')':
                    tokens.push_back(Token(RPAREN, ")"));
                    advance();
                    break;
                case '=':
                    tokens.push_back(makeEquals());
                    break;
                case ',':
                    tokens.push_back(Token(COMMA, ","));
                    advance();
                    break;
                case ':':
                    tokens.push_back(Token(COLON, ":"));
                    advance();
                    break;
                case '#':
                    tokens.push_back(Token(HASHTAG, "#"));
                    advance();
                    break;
                case '>':
                    tokens.push_back(makeGreaterThan());
                    break;
                case '<':
                    tokens.push_back(makeLessThan());
                    break;
                case '"':
                    tokens.push_back(Token(STRING, makeString()));
                    advance();
                    break;
                default:
                    cout << "Error: Invalid character encountered: " << currentChar() << endl;
                    exit(1);
                }
            }
            skipWhitespace();
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////
//                                  NODE
//////////////////////////////////////////////////////////////////////////////////
class Node
{
public:
    virtual ~Node() {}
    virtual void print() const = 0;
};
// store number as node
class NumberNode : public Node
{
public:
    int value;

    NumberNode(int value) : value(value) {}

    void print() const override
    {
        cout << value;
    }
};
// store binary operation which contains the left node, operation, and the right node
class BinOpNode : public Node
{
public:
    TokenType op;
    Node *leftNode;
    Node *rightNode;

    BinOpNode(TokenType op, Node *leftNode, Node *rightNode)
        : op(op), leftNode(leftNode), rightNode(rightNode) {}

    ~BinOpNode()
    {
        delete leftNode;
        delete rightNode;
    }

    void print() const override
    {
        cout << "(";
        leftNode->print();
        switch (op)
        {
        case PLUS:
            cout << " + ";
            break;
        case MINUS:
            cout << " - ";
            break;
        case MULTIPLY:
            cout << " * ";
            break;
        case DIVIDE:
            cout << " / ";
            break;
        case DOUBLE_EQUAL:
            cout << " == ";
            break;
        case LESS_THAN:
            cout << " < ";
            break;
        case LESS_THAN_OR_EQUAL_TO:
            cout << " <= ";
            break;
        case GREATER_THAN:
            cout << " > ";
            break;
        case GREATER_THAN_OR_EQUAL_TO:
            cout << " >= ";
            break;
        default:
            cout << " Unknown operator ";
            break;
        }
        rightNode->print();
        cout << ")";
    }
};
// node that store the variable name
class IdentifierNode : public Node
{
public:
    string name;

    IdentifierNode(const string &name) : name(name)
    {
        if (!isValidName(name))
        {
            throw std::invalid_argument("Invalid variable name: " + name);
        }
    }

    string getName() const
    {
        return name;
    }

    void print() const override
    {
        cout << name;
    }

private:
    // check if variable name match with the language grammar
    bool isValidName(const string &name) const
    {
        if (name.empty() || !isalpha(name[0]))
        {
            return false; // Must start with a letter
        }

        for (char c : name.substr(1))
        {
            if (!isalnum(c) && c)
            {
                return false; // Only letters, digits, and underscores are allowed after the first character
            }
        }

        return true;
    }
};
class LocalIdentifierNode : public Node
{
public:
    string name;

    LocalIdentifierNode(const string &name) : name(name)
    {
        if (!isValidName(name))
        {
            throw std::invalid_argument("Invalid variable name: " + name);
        }
    }

    string getName() const
    {
        return name;
    }

    void print() const override
    {
        cout << name;
    }

private:
    // check if variable name match with the language grammar
    bool isValidName(const string &name) const
    {
        if (name.empty() || !isalpha(name[0]))
        {
            return false; // Must start with a letter
        }

        for (char c : name.substr(1))
        {
            if (!isalnum(c) && c)
            {
                return false; // Only letters, digits, and underscores are allowed after the first character
            }
        }

        return true;
    }
};
// node that will store the variable name and the value
class AssignmentNode : public Node
{
public:
    IdentifierNode *variable;
    Node *expression;

    AssignmentNode(IdentifierNode *variable, Node *expression)
        : variable(variable), expression(expression) {}

    ~AssignmentNode()
    {
        delete variable;
        delete expression;
    }

    void print() const override
    {
        variable->print();
        cout << " = ";
        expression->print();
    }
};
class assignLocalVar : public Node
{
public:
    LocalIdentifierNode *variable;
    Node *expression;

    assignLocalVar(LocalIdentifierNode *variable, Node *expression)
        : variable(variable), expression(expression) {}

    ~assignLocalVar()
    {
        delete variable;
        delete expression;
    }

    void print() const override
    {
        variable->print();
        cout << " = ";
        expression->print();
    }
};
class accessLocalNode : public Node
{
private:
    string name;

public:
    accessLocalNode(const string &name) : name(name) {}

    void print() const override
    {
        cout << name;
    }

    string getName() const
    {
        return name;
    }
};
// this node is to use for accessing the value of the variable
class AccessNode : public Node
{
private:
    string name;

public:
    AccessNode(const string &name) : name(name) {}

    void print() const override
    {
        cout << name;
    }

    string getName() const
    {
        return name;
    }
};

class StringNode : public Node
{
private:
    string value;

public:
    StringNode(const string &value) : value(value) {}

    void print() const override
    {
        cout << "\"" << value << "\"";
    }

    const string &getValue() const
    {
        return value;
    }
};

class PrintNode : public Node
{
private:
    vector<Node *> arguments;

public:
    PrintNode(const vector<Node *> &arguments) : arguments(arguments) {}

    const vector<Node *> &getArguments() const
    {
        return arguments;
    }

    void print() const override
    {
        cout << "print(";
        for (size_t i = 0; i < arguments.size(); ++i)
        {
            arguments[i]->print();
            if (i != arguments.size() - 1)
                cout << ", ";
        }
        cout << ")";
    }
};

class ifCondition : public Node
{
private:
    vector<Node *> condition;

public:
    ifCondition(const std::vector<Node *> &condition)
        : condition(condition) {}

    const vector<Node *> &getCondition() const
    {
        return condition;
    }

    void print() const override
    {
        // Print body
        cout << "Body: ";
        for (const auto &condition : condition)
        {
            condition->print();
        }
        cout << ")";
    }
};

class func_init : public Node
{
public:
    IdentifierNode *func_name;
    vector<IdentifierNode *> parameters; // Change the type of parameters

    func_init(IdentifierNode *func_name, vector<IdentifierNode *> parameters)
        : func_name(func_name), parameters(parameters) {}

    ~func_init()
    {
        delete func_name;
        for (auto param : parameters)
        {
            delete param; // Deallocate memory for each parameter
        }
    }

    void print() const override
    {
        cout << "Function: " << func_name->getName() << "(";

        // Check if there are parameters to print
        if (!parameters.empty())
        {
            // Print the first parameter without a preceding comma
            cout << parameters[0]->getName();

            // Print the rest of the parameters with a preceding comma
            for (size_t i = 1; i < parameters.size(); ++i)
            {
                cout << ", " << parameters[i]->getName();
            }
        }

        cout << ")" << endl;
    }

    vector<IdentifierNode *> get_parameters() const
    {
        return parameters;
    }
};

class func_call : public Node
{
private:
    string func_name;
    vector<string> parameters;

public:
    func_call(const string &func_name, const vector<string> &parameters)
        : func_name(func_name), parameters(parameters) {}

    void print() const override
    {
        cout << func_name << " = ";
        for (const string &param : parameters)
        {
            cout << param << " ";
        }
    }

    string get_func_name() const
    {
        return func_name;
    }

    vector<string> get_parameters() const
    {
        return parameters;
    }
};

class returnNode : public Node
{
private:
    string returnLocalVar;
    string func_name;

public:
    returnNode(const string &returnLocalVar, const string &func_name)
        : returnLocalVar(returnLocalVar), func_name(func_name) {}

    void print() const override
    {
        cout << returnLocalVar;
        cout << " = ";
        cout << func_name;
    }

    string get_returnLocalVar() const
    {
        return returnLocalVar;
    }
    string get_func_name() const
    {
        return func_name;
    }
};
//////////////////////////////////////////////////////////////////////////////////
//                                  PARSER
//////////////////////////////////////////////////////////////////////////////////
// PARSER
class Parser
{
private:
    const vector<Token> &tokens;
    size_t currentTokenIndex;

public:
    Parser(const vector<Token> &tokens) : tokens(tokens), currentTokenIndex(0) {}

    Node *parse()
    {
        return expression();
    }

private:
    // expression will contain the term, operation, and another term
    Node *expression()
    {
        if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == PRINT)
        {
            // Parse the print function arguments
            vector<Node *> arguments;
            currentTokenIndex++; // Move past the "print" token

            if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == LPAREN)
            {
                currentTokenIndex++; // Move past the "(" token

                // Parse and tokenize print arguments
                while (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type != RPAREN)
                {
                    // Parse the argument and add it to the list of arguments
                    Node *argument = nullptr;
                    // Check the type of token to determine how to parse it
                    switch (tokens[currentTokenIndex].type)
                    {
                    case STRING:
                        argument = new StringNode(tokens[currentTokenIndex].value);
                        break;
                    case IDENTIFIER:
                        argument = new IdentifierNode(tokens[currentTokenIndex].value);
                        break;
                    // Handle other token types as needed
                    default:
                        cerr << "Error: Unexpected token type while parsing print arguments" << endl;
                        exit(1);
                    }
                    arguments.push_back(argument);

                    // Move to the next token
                    currentTokenIndex++;
                }

                currentTokenIndex++; // Move past the ")" token
                // Create and return a new PrintNode with the parsed arguments
                return new PrintNode(arguments);
            }
        }
        else if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == IF)
        {
            vector<Node *>
                conditions;
            currentTokenIndex++; // move past IF token so it will be at the left bracket

            if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == LEFT_BRACKET)
            {
                currentTokenIndex++; // move past the left bracket
                while (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type != COLON)
                {
                    Node *condition = expression();
                    conditions.push_back(condition);
                    currentTokenIndex++;
                    if (tokens[currentTokenIndex].type == RIGHT_BRACKET)
                    {
                        break;
                    }
                }
                // currentTokenIndex++; // Move past the "}" token
                return new ifCondition(conditions);
            }
        }
        else if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == DEF)
        {
            // Move past 'def'
            currentTokenIndex++; // identifier
            string func_name = tokens[currentTokenIndex].value;
            string parameter;
            currentTokenIndex++; // '('

            // Create a vector to store the parameters
            vector<IdentifierNode *> parameters;

            if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == LPAREN)
            {
                currentTokenIndex++; // Move past the left parenthesis

                while (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type != COLON)
                {
                    // Parse each parameter
                    parameter = tokens[currentTokenIndex].value;
                    parameters.push_back(new IdentifierNode(parameter));
                    currentTokenIndex++;

                    // Check for comma separator between parameters
                    if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == COMMA)
                    {
                        currentTokenIndex++; // Move past the comma
                    }
                    if (tokens[currentTokenIndex].type == RPAREN)
                    {
                        break;
                    }
                }

                return new func_init(new IdentifierNode(func_name), parameters); // Pass parameters by value
            }
        }

        else if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == CALL_FUNC)
        {
            string func_name = tokens[currentTokenIndex].value;
            currentTokenIndex++; // Move past the function name
            vector<string> parameters;

            if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == LPAREN)
            {
                currentTokenIndex++; // Move past the left parenthesis

                // Parse and tokenize the function arguments
                while (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type != RPAREN)
                {
                    string argument = tokens[currentTokenIndex].value;
                    parameters.push_back(argument);
                    currentTokenIndex++;

                    // Check for comma separator between arguments
                    if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == COMMA)
                    {
                        currentTokenIndex++; // Move past the comma
                    }
                }

                // Move past the right parenthesis
                if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == RPAREN)
                {
                    currentTokenIndex++;
                }

                return new func_call(func_name, parameters);
            }
        }

        else if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == RETURN)
        {
            currentTokenIndex++;                                     // move pass return token
            string returnLocalVar = tokens[currentTokenIndex].value; // get the return var
            currentTokenIndex++;
            string func_name = tokens[currentTokenIndex].value; // get the func_name
            return new returnNode(returnLocalVar, func_name);
        }
        else if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == LOCAL)
        {
            currentTokenIndex++;
            return nullptr;
        }
        // handle global variables
        else if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == IDENTIFIER && tokens[tokens.size() - 1].type != LOCAL)
        {
            // looking for equal sign so that it know that it should be a variable
            if (currentTokenIndex + 1 < tokens.size() && tokens[currentTokenIndex + 1].type == SINGLE_EQUAL)
            {
                string identifier = tokens[currentTokenIndex].value;
                currentTokenIndex += 2;
                Node *value = expression();
                // value->print();
                // cout << "\n";
                // assign that indentifier with its value
                return new AssignmentNode(new IdentifierNode(identifier), value);
            }
            // access node from variable that already defined and create an expression
            else
            {
                // Parse the identifier
                string identifier = tokens[currentTokenIndex++].value;
                // Check if there is a binary operation after the identifier
                if (currentTokenIndex < tokens.size() &&
                    (tokens[currentTokenIndex].type == PLUS || tokens[currentTokenIndex].type == MINUS ||
                     tokens[currentTokenIndex].type == MULTIPLY || tokens[currentTokenIndex].type == DIVIDE || tokens[currentTokenIndex].type == LESS_THAN ||
                     tokens[currentTokenIndex].type == GREATER_THAN || tokens[currentTokenIndex].type == LESS_THAN_OR_EQUAL_TO ||
                     tokens[currentTokenIndex].type == GREATER_THAN_OR_EQUAL_TO || tokens[currentTokenIndex].type == DOUBLE_EQUAL))
                {
                    // Save the operation type
                    TokenType opType = tokens[currentTokenIndex++].type;
                    // Parse the right-hand side of the expression
                    Node *right = term();
                    // Create a BinOpNode with the identifier as the left operand, operation, and right-hand side
                    Node *result = new BinOpNode(opType, new AccessNode(identifier), right);
                    // Parse any subsequent operations and their operands
                    while (currentTokenIndex < tokens.size() &&
                           (tokens[currentTokenIndex].type == PLUS || tokens[currentTokenIndex].type == MINUS || tokens[currentTokenIndex].type == LESS_THAN ||
                            tokens[currentTokenIndex].type == GREATER_THAN || tokens[currentTokenIndex].type == LESS_THAN_OR_EQUAL_TO ||
                            tokens[currentTokenIndex].type == GREATER_THAN_OR_EQUAL_TO || tokens[currentTokenIndex].type == DOUBLE_EQUAL))
                    {
                        // Save the operation type
                        opType = tokens[currentTokenIndex++].type;

                        // Parse the next term
                        right = term();
                        // Update the result with the new operation and right-hand side
                        result = new BinOpNode(opType, result, right);
                    }
                    return result;
                }
                // If there is no binary operation, treat it as a simple variable access
                else
                {
                    return new AccessNode(identifier);
                }
            }
        }
        // handle local variables
        else if (currentTokenIndex < tokens.size() && tokens[currentTokenIndex].type == IDENTIFIER && tokens[tokens.size() - 1].type == LOCAL)
        {
            // looking for equal sign so that it know that it should be a variable
            if (currentTokenIndex + 1 < tokens.size() && tokens[currentTokenIndex + 1].type == SINGLE_EQUAL)
            {
                string identifier = tokens[currentTokenIndex].value;
                currentTokenIndex += 2;
                Node *value = expression();
                // assign that indentifier with its value
                return new assignLocalVar(new LocalIdentifierNode(identifier), value);
            }
            // access node from variable that already defined and create an expression
            else
            {
                // Parse the identifier
                string identifier = tokens[currentTokenIndex++].value;

                // Check if there is a binary operation after the identifier
                if (currentTokenIndex < tokens.size() &&
                    (tokens[currentTokenIndex].type == PLUS || tokens[currentTokenIndex].type == MINUS ||
                     tokens[currentTokenIndex].type == MULTIPLY || tokens[currentTokenIndex].type == DIVIDE || tokens[currentTokenIndex].type == LESS_THAN ||
                     tokens[currentTokenIndex].type == GREATER_THAN || tokens[currentTokenIndex].type == LESS_THAN_OR_EQUAL_TO ||
                     tokens[currentTokenIndex].type == GREATER_THAN_OR_EQUAL_TO || tokens[currentTokenIndex].type == DOUBLE_EQUAL))
                {
                    // Save the operation type
                    TokenType opType = tokens[currentTokenIndex++].type;
                    // Parse the right-hand side of the expression
                    Node *right = term();
                    // Create a BinOpNode with the identifier as the left operand, operation, and right-hand side
                    Node *result = new BinOpNode(opType, new accessLocalNode(identifier), right);
                    // Parse any subsequent operations and their operands
                    while (currentTokenIndex < tokens.size() &&
                           (tokens[currentTokenIndex].type == PLUS || tokens[currentTokenIndex].type == MINUS || tokens[currentTokenIndex].type == LESS_THAN ||
                            tokens[currentTokenIndex].type == GREATER_THAN || tokens[currentTokenIndex].type == LESS_THAN_OR_EQUAL_TO ||
                            tokens[currentTokenIndex].type == GREATER_THAN_OR_EQUAL_TO || tokens[currentTokenIndex].type == DOUBLE_EQUAL))
                    {
                        // Save the operation type
                        opType = tokens[currentTokenIndex++].type;

                        // Parse the next term
                        right = term();

                        // Update the result with the new operation and right-hand side
                        result = new BinOpNode(opType, result, right);
                    }
                    return result;
                }
                // If there is no binary operation, treat it as a simple variable access
                else
                {
                    return new accessLocalNode(identifier);
                }
            }
        }
        Node *result = term();
        // this will parse if the token is not an identifier (i.e. 3 + 2 + 3) group it in a expression ((3+2)+3)
        while (currentTokenIndex < tokens.size() &&
               (tokens[currentTokenIndex].type == PLUS || tokens[currentTokenIndex].type == MINUS || tokens[currentTokenIndex].type == LESS_THAN ||
                tokens[currentTokenIndex].type == GREATER_THAN || tokens[currentTokenIndex].type == LESS_THAN_OR_EQUAL_TO ||
                tokens[currentTokenIndex].type == GREATER_THAN_OR_EQUAL_TO || tokens[currentTokenIndex].type == DOUBLE_EQUAL))
        {
            TokenType opType = tokens[currentTokenIndex++].type;
            Node *right = term();
            result = new BinOpNode(opType, result, right);
        }
        return result;
    }

    // a term can contain a factor or a factor follow by an operation sign and another factor
    Node *term()
    {
        Node *result = factor();
        while (currentTokenIndex < tokens.size() &&
               (tokens[currentTokenIndex].type == MULTIPLY || tokens[currentTokenIndex].type == DIVIDE))
        {
            TokenType opType = tokens[currentTokenIndex++].type;
            Node *right = factor();
            result = new BinOpNode(opType, result, right);
        }

        return result;
    }

    Node *factor()
    {
        Token currentToken = tokens[currentTokenIndex++];
        if (currentToken.type == NUMBER)
        {
            return new NumberNode(stoi(currentToken.value));
        }
        else if (currentToken.type == MINUS)
        {
            currentToken = tokens[currentTokenIndex++];
            int negativeVal = stoi(currentToken.value) * -1;
            return new NumberNode(negativeVal);
        }
        else if (currentToken.type == IDENTIFIER && tokens[tokens.size() - 1].type != LOCAL)
        {
            return new IdentifierNode(currentToken.value);
        }
        else if (currentToken.type == IDENTIFIER && tokens[tokens.size() - 1].type == LOCAL)
        {
            return new LocalIdentifierNode(currentToken.value);
        }
        else if (currentToken.type == STRING)
        {
            return new StringNode(currentToken.value);
        }
        else if (currentToken.type == CALL_FUNC)
        {
            string func_name = currentToken.value;
            currentToken = tokens[currentTokenIndex++];
            vector<string> parameters;

            if (currentToken.type == LPAREN)
            {
                currentToken = tokens[currentTokenIndex++];
                // Parse and tokenize the function arguments
                while (currentToken.type != RPAREN)
                {
                    string argument = currentToken.value;
                    parameters.push_back(argument);
                    currentToken = tokens[currentTokenIndex++];

                    // Check for comma separator between arguments
                    if (currentToken.type == COMMA)
                    {
                        currentToken = tokens[currentTokenIndex++];
                    }
                }

                // Move past the right parenthesis
                if (currentToken.type == RPAREN)
                {
                    currentToken = tokens[currentTokenIndex++];
                }
                return new func_call(func_name, parameters);
            }
        }
        else if (currentToken.type == LPAREN)
        {
            // If it's not a function call, parse the expression within parentheses
            Node *result = expression();
            if (tokens[currentTokenIndex++].type != RPAREN)
            {
                cerr << "Error: Expected closing parenthesis ')'" << endl;
                exit(1);
            }
            return result;
        }
        else
        {
            cerr << "Error: Invalid token encountered." << currentToken.value << endl;
            exit(1);
        }
        return nullptr;
    }
};

//////////////////////////////////////////////////////////////////////////////////
//                                  INTERPRETER
//////////////////////////////////////////////////////////////////////////////////
class Interpreter
{
public:
    static int evaluate(Node *root, SymbolTable &symbolTable)
    {
        if (root == nullptr)
        {
            cerr << "Error: Empty AST" << endl;
            exit(1);
        }

        return evaluateNode(root, symbolTable);
    }

private:
    // this will evaluate the parse list
    static int evaluateNode(Node *node, SymbolTable &symbolTable)
    {
        if (NumberNode *numberNode = dynamic_cast<NumberNode *>(node))
        {
            return numberNode->value;
        }
        else if (IdentifierNode *identifierNode = dynamic_cast<IdentifierNode *>(node))
        {
            return symbolTable.getGlobalVar(identifierNode->getName());
        }
        else if (LocalIdentifierNode *localIdenNode = dynamic_cast<LocalIdentifierNode *>(node))
        {
            if (symbolTable.isInLocalList(localIdenNode->getName()))
            {
                return symbolTable.getLocalVar(localIdenNode->getName());
            }
            else if (!symbolTable.isInLocalList(localIdenNode->getName()) && symbolTable.isInGlobalList(localIdenNode->getName()))
            {
                return symbolTable.getGlobalVar(localIdenNode->getName());
            }
        }
        else if (BinOpNode *binOpNode = dynamic_cast<BinOpNode *>(node))
        {
            int leftValue = evaluateNode(binOpNode->leftNode, symbolTable);
            int rightValue = evaluateNode(binOpNode->rightNode, symbolTable);

            switch (binOpNode->op)
            {
            case PLUS:
                return leftValue + rightValue;
            case MINUS:
                return leftValue - rightValue;
            case MULTIPLY:
                return leftValue * rightValue;
            case DIVIDE:
                if (rightValue == 0)
                {
                    cerr << "Error: Division by zero" << endl;
                    exit(1);
                }
                return leftValue / rightValue;
            // return 1 for true and 0 for false
            case DOUBLE_EQUAL:
                return leftValue == rightValue;
            case LESS_THAN:
                return leftValue < rightValue;
            case LESS_THAN_OR_EQUAL_TO:
                return leftValue <= rightValue;
            case GREATER_THAN:
                return leftValue > rightValue;
            case GREATER_THAN_OR_EQUAL_TO:
                return leftValue >= rightValue;
            default:
                cerr << "Error: Unknown operator" << endl;
                exit(1);
            }
        }
        else if (AccessNode *accessNode = dynamic_cast<AccessNode *>(node))
        {
            return symbolTable.getGlobalVar(accessNode->getName());
        }
        else if (accessLocalNode *accessLocal = dynamic_cast<accessLocalNode *>(node))
        {
            if (symbolTable.isInLocalList(accessLocal->getName()))
            {
                return symbolTable.getLocalVar(accessLocal->getName());
            }
            else if (!symbolTable.isInLocalList(accessLocal->getName()) && symbolTable.isInGlobalList(accessLocal->getName()))
            {
                return symbolTable.getGlobalVar(accessLocal->getName());
            }
        }
        else if (AssignmentNode *assignmentNode = dynamic_cast<AssignmentNode *>(node))
        {
            int assignedValue = evaluateNode(assignmentNode->expression, symbolTable);
            symbolTable.addGlobalVar(assignmentNode->variable->getName(), assignedValue);
            return assignedValue;
        }
        else if (assignLocalVar *assignmentNode = dynamic_cast<assignLocalVar *>(node))
        {
            int assignedValue = evaluateNode(assignmentNode->expression, symbolTable);
            // cout << "local: " << assignedValue << endl;
            symbolTable.addLocalVar(assignmentNode->variable->getName(), assignedValue);
            return assignedValue;
        }
        // this node is for the print function
        else if (PrintNode *printNode = dynamic_cast<PrintNode *>(node))
        {
            const vector<Node *> &arguments = printNode->getArguments();
            for (size_t i = 0; i < arguments.size(); ++i)
            {
                Node *argument = arguments[i];
                if (StringNode *stringArg = dynamic_cast<StringNode *>(argument))
                {
                    cout << stringArg->getValue();
                }
                else if (NumberNode *numberArg = dynamic_cast<NumberNode *>(argument))
                {
                    cout << numberArg->value;
                }
                else
                {
                    // Evaluate and print other types of nodes
                    int value = evaluateNode(argument, symbolTable);
                    cout << value;
                }

                // Print a space after each argument except for the last one
                if (i < arguments.size() - 1)
                    cout << " ";
            }
            cout << endl; // Print newline after printing all arguments
            return 0;
        }
        else if (ifCondition *conditionNode = dynamic_cast<ifCondition *>(node))
        {
            const vector<Node *> &conditions = conditionNode->getCondition();
            bool result = true; // Default result is true

            for (Node *condition : conditions)
            {
                int conditionValue = evaluateNode(condition, symbolTable);
                if (conditionValue == 0) // If any condition evaluates to false, set result to false
                {
                    result = false;
                    break;
                }
            }

            return result ? 1 : 0; // Convert bool result to integer (1 for true, 0 for false)
        }
        else if (func_init *func_node = dynamic_cast<func_init *>(node))
        {
            // Get the function name and its parameters
            string func_name = func_node->func_name->getName();
            vector<string> param_names; // Vector to store parameter names

            for (const IdentifierNode *param_ptr : func_node->get_parameters())
            {
                const IdentifierNode &param = *param_ptr;
                param_names.push_back(param.getName());
            }
            symbolTable.addFuncInit(func_name, param_names);
            for (const IdentifierNode *param_ptr : func_node->get_parameters())
            {
                const IdentifierNode &param = *param_ptr;
                int param_init = 0;
                symbolTable.addLocalVar(param.getName(), param_init);
            }
            return 0;
        }

        else if (func_call *func_node = dynamic_cast<func_call *>(node))
        {
            int funcname_init = 0;
            int index = 0;
            vector<string> parameters = func_node->get_parameters();

            for (const auto &param : parameters)
            {
                int reAssignVal;
                // int reAssignVal = symbolTable.getGlobalVar(param);
                if (symbolTable.isInGlobalList(param))
                {
                    reAssignVal = symbolTable.getGlobalVar(param);
                }
                else if (symbolTable.isInLocalList(param))
                {
                    reAssignVal = symbolTable.getLocalVar(param);
                }
                string func_name = func_node->get_func_name();
                string param_init = symbolTable.getFunc(func_name);
                if (param_init.find(",") != std::string::npos)
                {
                    string paramChar(1, param_init[index]);
                    symbolTable.addLocalVar(paramChar, reAssignVal);
                    index++;
                    if (param_init[index] == ',')
                    {
                        index++;
                    }
                }
                else
                {
                    symbolTable.addLocalVar(param_init, reAssignVal);
                }
            }
            symbolTable.addGlobalVar(func_node->get_func_name(), funcname_init);
            return 0;
        }
        else if (returnNode *return_node = dynamic_cast<returnNode *>(node))
        {
            int reAssignVal = symbolTable.getLocalVar(return_node->get_returnLocalVar());
            symbolTable.addGlobalVar(return_node->get_func_name(), reAssignVal);
            return 0;
        }
        else
        {
            cerr << "Error: Unexpected node" << endl;
            exit(1);
        }
        return 0;
    }
};

//////////////////////////////////////////////////////////////////////////////////
//                                  READ FILE
//////////////////////////////////////////////////////////////////////////////////
string readFile(const string &fileName)
{
    ifstream file(fileName);
    if (!file)
    {
        cerr << "Unable to open file: " << fileName << endl;
        exit(1);
    }

    string content;
    string line;
    bool copied = false; // Flag to track if a line has been copied

    while (getline(file, line))
    {
        // Check if the line satisfies the condition
        if (!line.empty() &&
            line.find("=") != string::npos &&
            line.find("(") != string::npos &&
            line.find(")") != string::npos &&
            line.find("print") == string::npos)
        {
            // If the condition is met and a line has not been copied yet, make a copy of it and add it below
            if (!copied)
            {
                // Copy the line with content inside parentheses trimmed
                string copiedLine = line;
                size_t startPos = copiedLine.find("(");
                size_t endPos = copiedLine.find(")");
                copiedLine.erase(startPos, endPos - startPos + 1);

                // Add the trimmed copy below the original line
                content += line + '\n';
                content += copiedLine + '\n';
                copied = false;
            }
            else
            {
                // Add the original line
                content += line + '\n';
            }
        }
        else
        {
            // If the line doesn't meet the condition, add it as it is
            content += line + '\n';
        }
    }

    file.close();
    return content;
}
//////////////////////////////////////////////////////////////////////////////////
//                                  MAIN
//////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{

    // if (argc != 2)
    // {
    //     cerr << "Usage: " << argv[0] << " <filename>" << endl;
    //     return 1;
    // }

    // // Extract the filename from command-line arguments
    // string fileName = argv[1];
    string fileName = "/Users/joel/Desktop/School/2024spring/ProgrammingLanguages/hwwithvariables/4315-HW2/test.py";
    string input = readFile(fileName);

    // Create a symbol table
    SymbolTable symbolTable;

    // Initialize the flags to indicate if the next line should be skipped
    bool skipStatement = false;
    bool elseCase = false;
    // innitialize the flags to indicate if the it's first or second level if else statement (2 levels)
    bool first_lv_if = false;
    bool first_lv_else = false;
    bool second_lv_if = false;
    bool second_lv_else = false;
    // set flag if the next lines belong to a function
    bool function_block = false;
    string func_name;
    stringstream func_block;
    // Split the input into lines
    stringstream ss(input);
    string line;
    vector<size_t> subPosition;
    unordered_map<string, string> funcBlockMap;
    // cout << ss.str() << endl;
    while (getline(ss, line))
    {
        // Skip empty lines or lines containing only comments
        if (!line.empty() && line[0] == ' ' && line.find_first_not_of(" \t") != std::string::npos && line.find_first_not_of(" \t") == line.find('#'))
        {
            // The line starts with a white space and is a comment
            continue;
        }
        if (function_block)
        {
            if (line.empty())
            {
                func_block << line << endl;
                continue;
            }
            else if (!line.empty() && line[0] == ' ' && line.find("return") == std::string::npos && line.find("else") == std::string::npos && line.find("if") == std::string::npos && line.find("=") == std::string::npos)
            {
                func_block << line << endl;
                continue;
            }
            else if (!line.empty() && line.find("return") != std::string::npos)
            {
                string trimmed_line = line.substr(subPosition[0]);
                func_block << trimmed_line << " " << func_name << endl;
                continue;
            }
            else if (!line.empty() && line[0] == ' ' && line.find("return") == std::string::npos && line.find("else") == std::string::npos)
            {
                size_t position = line.find_first_not_of(" \t");
                subPosition.push_back(position);
                string trimmed_line = line.substr(subPosition[0]);
                func_block << trimmed_line << " local" << endl;
                continue;
            }
            else if (!line.empty() && line.find("else") != std::string::npos)
            {
                string trimmed_line = line.substr(subPosition[0]);
                func_block << trimmed_line << endl;
                continue;
            }
            else
            {
                string stored_function_block = func_block.str();
                funcBlockMap[func_name] = stored_function_block;
                func_block.str("");
                function_block = false;
            }
            // cout << func_block.str() << endl;
        }
        // skip first level if else statement
        if (skipStatement == true && (first_lv_if || first_lv_else) && !second_lv_if && !second_lv_else)
        {
            // cout << line << endl;
            if (!line.empty() && line[0] == ' ')
            {
                continue;
            }
            else
            {
                skipStatement = false;
            }
        }
        // set first level if flag to true
        else if (!line.empty() && line[0] == 'i' && line[1] == 'f' && !first_lv_if)
        {
            // cout << line << endl;
            first_lv_if = true;
            first_lv_else = false;
            second_lv_if = false;
            second_lv_else = false;
        }
        // set first level else flag to true
        else if (!line.empty() && line[0] == 'e' && line[1] == 'l' && line[2] == 's' && line[3] == 'e' && !first_lv_else)
        {
            // cout << line << endl;
            first_lv_else = true;
            first_lv_if = false;
            second_lv_if = false;
            second_lv_else = false;
        }
        // set second level if flag to true
        else if (!line.empty() && line[0] == ' ' && line[4] == 'i' && line[5] == 'f')
        {
            // cout << line << endl;
            second_lv_if = true;
            second_lv_else = false;
        }
        // set second level else flag to true
        else if (!line.empty() && line[0] == ' ' && line[4] == 'e' && line[5] == 'l' && line[6] == 's' && line[7] == 'e')
        {
            // cout << line << endl;
            second_lv_else = true;
            second_lv_if = false;
        }
        // skip second level if
        if (skipStatement == true && second_lv_if)
        {
            if (!line.empty() && line[0] == ' ' && !second_lv_else)
            {
                // cout << line << endl;
                continue;
            }
            else
            {
                skipStatement = false;
            }
        }
        // skip second level else
        if (skipStatement == true && second_lv_else)
        {
            // cout << line << endl;
            if (!line.empty() && line[0] == ' ' && line[4] == ' ' && !second_lv_if)
            {
                // cout << line << endl;
                continue;
            }
            else
            {
                skipStatement = false;
            }
        }
        // Tokenize the line
        Lexer lexer(line);
        lexer.tokenize(symbolTable);
        const vector<Token> &tokens = lexer.getTokens();
        // for (const Token &token : tokens)
        // {
        //     cout << "value: " << token.value << " Type: " << token.type << endl;
        // }

        // Parse and evaluate the tokens only if there are tokens to parse
        if (!tokens.empty())
        {
            // Check if the first token is an IF token (the first level)
            if (tokens[0].type == IF)
            {
                // Parse the tokens
                Parser parser(tokens);
                Node *ast = parser.parse();
                if (ast == nullptr)
                {
                    cerr << "Error parsing line: " << line << endl;
                    return 1;
                }

                // Evaluate the AST with the symbol table
                bool result = Interpreter::evaluate(ast, symbolTable);
                if (!result)
                {
                    // Set flag to skip if statement
                    // else it will continue and skipStatement = false;
                    skipStatement = true;
                    elseCase = true;
                }
                delete ast;
            }
            else if (tokens[0].type == ELSE)
            {
                if (elseCase == true)
                {
                    elseCase = false;
                    continue;
                }
                // skip else statement if it's not an else case
                else
                {
                    skipStatement = true;
                }
            }
            else if (tokens[0].type == DEF)
            {
                function_block = true;
                func_name = tokens[1].value;
                Parser parser(tokens);
                Node *ast = parser.parse();
                if (ast == nullptr)
                {
                    cerr << "Error parsing line: " << line << endl;
                    return 1;
                }
                // Evaluate the AST with the symbol table
                Interpreter::evaluate(ast, symbolTable);
                // Delete the AST
                delete ast;
            }
            else if (tokens[2].type == CALL_FUNC || tokens[4].type == CALL_FUNC || tokens[6].type == CALL_FUNC || tokens[8].type == CALL_FUNC || tokens[10].type == CALL_FUNC || tokens[12].type == CALL_FUNC)
            {
                string func_name;
                if (tokens[2].type == CALL_FUNC)
                {
                    func_name = tokens[2].value;
                }
                else if (tokens[4].type == CALL_FUNC)
                {
                    func_name = tokens[4].value;
                }
                else if (tokens[6].type == CALL_FUNC)
                {
                    func_name = tokens[6].value;
                }
                else if (tokens[8].type == CALL_FUNC)
                {
                    func_name = tokens[8].value;
                }
                else if (tokens[10].type == CALL_FUNC)
                {
                    func_name = tokens[10].value;
                }
                else if (tokens[12].type == CALL_FUNC)
                {
                    func_name = tokens[12].value;
                }
                stringstream newFuncBlock(funcBlockMap[func_name]);
                Parser parser(tokens);
                Node *ast = parser.parse();
                if (ast == nullptr)
                {
                    cerr << "Error parsing line: " << line << endl;
                    return 1;
                }
                // Evaluate the AST with the symbol table
                Interpreter::evaluate(ast, symbolTable);
                // Delete the AST
                delete ast;

                bool localSkipStatement = false;
                bool localElseCase = false;
                // innitialize the flags to indicate if the it's first or second level if else statement (2 levels)
                bool local_first_lv_if = false;
                bool local_first_lv_else = false;
                bool local_second_lv_if = false;
                bool local_second_lv_else = false;

                while (getline(newFuncBlock, line))
                {
                    // Skip empty lines or lines containing only comments
                    if (!line.empty() && line[0] == ' ' && line.find_first_not_of(" \t") != std::string::npos && line.find_first_not_of(" \t") == line.find('#'))
                    {
                        // The line starts with a white space and is a comment
                        continue;
                    }
                    // skip first level if else statement
                    if (localSkipStatement == true && (local_first_lv_if || local_first_lv_else) && !local_second_lv_if && !local_second_lv_else)
                    {
                        // cout << line << endl;
                        if (!line.empty() && line[0] == ' ')
                        {
                            continue;
                        }
                        else
                        {
                            localSkipStatement = false;
                        }
                    }
                    // set first level if flag to true
                    else if (!line.empty() && line[0] == 'i' && line[1] == 'f' && !local_first_lv_if)
                    {
                        // cout << line << endl;
                        local_first_lv_if = true;
                        local_first_lv_else = false;
                        local_second_lv_if = false;
                        local_second_lv_else = false;
                    }
                    // set first level else flag to true
                    else if (!line.empty() && line[0] == 'e' && line[1] == 'l' && line[2] == 's' && line[3] == 'e' && !local_first_lv_else)
                    {
                        // cout << line << endl;
                        local_first_lv_else = true;
                        local_first_lv_if = false;
                        local_second_lv_if = false;
                        local_second_lv_else = false;
                    }
                    // set second level if flag to true
                    else if (!line.empty() && line[0] == ' ' && line[4] == 'i' && line[5] == 'f')
                    {
                        // cout << line << endl;
                        local_second_lv_if = true;
                        local_second_lv_else = false;
                    }
                    // set second level else flag to true
                    else if (!line.empty() && line[0] == ' ' && line[4] == 'e' && line[5] == 'l' && line[6] == 's' && line[7] == 'e')
                    {
                        // cout << line << endl;
                        local_second_lv_else = true;
                        local_second_lv_if = false;
                    }
                    // skip second level if
                    if (localSkipStatement == true && local_second_lv_if)
                    {
                        if (!line.empty() && line[0] == ' ' && !local_second_lv_else)
                        {
                            // cout << line << endl;
                            continue;
                        }
                        else
                        {
                            localSkipStatement = false;
                        }
                    }
                    // skip second level else
                    if (localSkipStatement == true && local_second_lv_else)
                    {
                        // cout << line << endl;
                        if (!line.empty() && line[0] == ' ' && line[4] == ' ' && !local_second_lv_if)
                        {
                            // cout << line << endl;
                            continue;
                        }
                        else
                        {
                            localSkipStatement = false;
                        }
                    }
                    // Tokenize the line
                    Lexer lexer(line);
                    lexer.tokenize(symbolTable);
                    const vector<Token> &tokens = lexer.getTokens();
                    // for (const Token &token : tokens)
                    // {
                    //     cout << "value: " << token.value << " Type: " << token.type << endl;
                    // }

                    // Parse and evaluate the tokens only if there are tokens to parse
                    if (!tokens.empty())
                    {
                        // Check if the first token is an IF token (the first level)
                        if (tokens[0].type == IF)
                        {
                            // Parse the tokens
                            Parser parser(tokens);
                            Node *ast = parser.parse();
                            if (ast == nullptr)
                            {
                                cerr << "Error parsing line: " << line << endl;
                                return 1;
                            }

                            // Evaluate the AST with the symbol table
                            bool result = Interpreter::evaluate(ast, symbolTable);
                            if (!result)
                            {
                                // Set flag to skip if statement
                                // else it will continue and localSkipStatement = false;
                                localSkipStatement = true;
                                localElseCase = true;
                            }
                            delete ast;
                        }
                        else if (tokens[0].type == ELSE)
                        {
                            if (localElseCase == true)
                            {
                                localElseCase = false;
                                continue;
                            }
                            // skip else statement if it's not an else case
                            else
                            {
                                localSkipStatement = true;
                            }
                        }
                        else if (tokens[0].type == RETURN)
                        {
                            // Parse the tokens
                            Parser parser(tokens);
                            Node *ast = parser.parse();
                            if (ast == nullptr)
                            {
                                cerr << "Error parsing line: " << line << endl;
                                return 1;
                            }
                            // Evaluate the AST with the symbol table
                            Interpreter::evaluate(ast, symbolTable);
                            // symbolTable.printLocalVars();
                            // symbolTable.printFuncInit();
                            // Delete the AST
                            delete ast;
                        }
                        else if (tokens[2].type == CALL_FUNC || tokens[4].type == CALL_FUNC || tokens[6].type == CALL_FUNC || tokens[8].type == CALL_FUNC || tokens[10].type == CALL_FUNC || tokens[12].type == CALL_FUNC)
                        {
                            string func_name;
                            if (tokens[2].type == CALL_FUNC)
                            {
                                func_name = tokens[2].value;
                            }
                            else if (tokens[4].type == CALL_FUNC)
                            {
                                func_name = tokens[4].value;
                            }
                            else if (tokens[6].type == CALL_FUNC)
                            {
                                func_name = tokens[6].value;
                            }
                            else if (tokens[8].type == CALL_FUNC)
                            {
                                func_name = tokens[8].value;
                            }
                            else if (tokens[10].type == CALL_FUNC)
                            {
                                func_name = tokens[10].value;
                            }
                            else if (tokens[12].type == CALL_FUNC)
                            {
                                func_name = tokens[12].value;
                            }
                            stringstream newFuncBlock(funcBlockMap[func_name]);
                            Parser parser(tokens);
                            Node *ast = parser.parse();
                            if (ast == nullptr)
                            {
                                cerr << "Error parsing line: " << line << endl;
                                return 1;
                            }
                            // Evaluate the AST with the symbol table
                            Interpreter::evaluate(ast, symbolTable);
                            // Delete the AST
                            delete ast;

                            bool localSkipStatement = false;
                            bool localElseCase = false;
                            // innitialize the flags to indicate if the it's first or second level if else statement (2 levels)
                            bool local_first_lv_if = false;
                            bool local_first_lv_else = false;
                            bool local_second_lv_if = false;
                            bool local_second_lv_else = false;

                            while (getline(newFuncBlock, line))
                            {
                                // Skip empty lines or lines containing only comments
                                if (!line.empty() && line[0] == ' ' && line.find_first_not_of(" \t") != std::string::npos && line.find_first_not_of(" \t") == line.find('#'))
                                {
                                    // The line starts with a white space and is a comment
                                    continue;
                                }
                                // skip first level if else statement
                                if (localSkipStatement == true && (local_first_lv_if || local_first_lv_else) && !local_second_lv_if && !local_second_lv_else)
                                {
                                    // cout << line << endl;
                                    if (!line.empty() && line[0] == ' ')
                                    {
                                        continue;
                                    }
                                    else
                                    {
                                        localSkipStatement = false;
                                    }
                                }
                                // set first level if flag to true
                                else if (!line.empty() && line[0] == 'i' && line[1] == 'f' && !local_first_lv_if)
                                {
                                    // cout << line << endl;
                                    local_first_lv_if = true;
                                    local_first_lv_else = false;
                                    local_second_lv_if = false;
                                    local_second_lv_else = false;
                                }
                                // set first level else flag to true
                                else if (!line.empty() && line[0] == 'e' && line[1] == 'l' && line[2] == 's' && line[3] == 'e' && !local_first_lv_else)
                                {
                                    // cout << line << endl;
                                    local_first_lv_else = true;
                                    local_first_lv_if = false;
                                    local_second_lv_if = false;
                                    local_second_lv_else = false;
                                }
                                // set second level if flag to true
                                else if (!line.empty() && line[0] == ' ' && line[4] == 'i' && line[5] == 'f')
                                {
                                    // cout << line << endl;
                                    local_second_lv_if = true;
                                    local_second_lv_else = false;
                                }
                                // set second level else flag to true
                                else if (!line.empty() && line[0] == ' ' && line[4] == 'e' && line[5] == 'l' && line[6] == 's' && line[7] == 'e')
                                {
                                    // cout << line << endl;
                                    local_second_lv_else = true;
                                    local_second_lv_if = false;
                                }
                                // skip second level if
                                if (localSkipStatement == true && local_second_lv_if)
                                {
                                    if (!line.empty() && line[0] == ' ' && !local_second_lv_else)
                                    {
                                        // cout << line << endl;
                                        continue;
                                    }
                                    else
                                    {
                                        localSkipStatement = false;
                                    }
                                }
                                // skip second level else
                                if (localSkipStatement == true && local_second_lv_else)
                                {
                                    // cout << line << endl;
                                    if (!line.empty() && line[0] == ' ' && line[4] == ' ' && !local_second_lv_if)
                                    {
                                        // cout << line << endl;
                                        continue;
                                    }
                                    else
                                    {
                                        localSkipStatement = false;
                                    }
                                }
                                // Tokenize the line
                                Lexer lexer(line);
                                lexer.tokenize(symbolTable);
                                const vector<Token> &tokens = lexer.getTokens();
                                // for (const Token &token : tokens)
                                // {
                                //     cout << token.value << endl;
                                // }

                                // Parse and evaluate the tokens only if there are tokens to parse
                                if (!tokens.empty())
                                {
                                    // Check if the first token is an IF token (the first level)
                                    if (tokens[0].type == IF)
                                    {
                                        // Parse the tokens
                                        Parser parser(tokens);
                                        Node *ast = parser.parse();
                                        if (ast == nullptr)
                                        {
                                            cerr << "Error parsing line: " << line << endl;
                                            return 1;
                                        }

                                        // Evaluate the AST with the symbol table
                                        bool result = Interpreter::evaluate(ast, symbolTable);
                                        if (!result)
                                        {
                                            // Set flag to skip if statement
                                            // else it will continue and localSkipStatement = false;
                                            localSkipStatement = true;
                                            localElseCase = true;
                                        }
                                        delete ast;
                                    }
                                    else if (tokens[0].type == ELSE)
                                    {
                                        if (localElseCase == true)
                                        {
                                            localElseCase = false;
                                            continue;
                                        }
                                        // skip else statement if it's not an else case
                                        else
                                        {
                                            localSkipStatement = true;
                                        }
                                    }
                                    else if (tokens[0].type == RETURN)
                                    {
                                        // Parse the tokens
                                        Parser parser(tokens);
                                        Node *ast = parser.parse();
                                        if (ast == nullptr)
                                        {
                                            cerr << "Error parsing line: " << line << endl;
                                            return 1;
                                        }
                                        // Evaluate the AST with the symbol table
                                        Interpreter::evaluate(ast, symbolTable);
                                        // symbolTable.printLocalVars();
                                        // symbolTable.printFuncInit();
                                        // Delete the AST
                                        delete ast;
                                    }
                                    // parse and evaluate if not if or else statement
                                    else
                                    {
                                        // Parse the tokens
                                        Parser parser(tokens);
                                        Node *ast = parser.parse();
                                        if (ast == nullptr)
                                        {
                                            cerr << "Error parsing line: " << line << endl;
                                            return 1;
                                        }
                                        // Evaluate the AST with the symbol table
                                        Interpreter::evaluate(ast, symbolTable);

                                        // Delete the AST
                                        delete ast;
                                    }
                                }
                            }
                        }
                        // parse and evaluate if not if or else statement
                        else
                        {
                            // Parse the tokens
                            Parser parser(tokens);
                            Node *ast = parser.parse();
                            if (ast == nullptr)
                            {
                                cerr << "Error parsing line: " << line << endl;
                                return 1;
                            }
                            // Evaluate the AST with the symbol table
                            Interpreter::evaluate(ast, symbolTable);

                            // Delete the AST
                            delete ast;
                        }
                    }
                }
            }
            // parse and evaluate if not if or else statement
            else
            {
                // Parse the tokens
                Parser parser(tokens);
                Node *ast = parser.parse();
                if (ast == nullptr)
                {
                    cerr << "Error parsing line: " << line << endl;
                    return 1;
                }
                // Evaluate the AST with the symbol table
                Interpreter::evaluate(ast, symbolTable);
                // symbolTable.printGlobalVars();

                // Delete the AST
                delete ast;
            }
        }
    }

    return 0;
}
