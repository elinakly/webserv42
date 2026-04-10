#include "ConfigParser.hpp"

ASTNode* ConfigParser::parseLocation()
{
    std::string word = expectWord();
    if (word != "location")
        throw std::runtime_error("Expected 'location'");

    std::string path = expectWord();
    expect(LBRACE);
    LocationNode* location = new LocationNode(path);

    while (peek().type != RBRACE)
    {
        if (peek().type == END)
            throw std::runtime_error("Unexpected end in location block");
        if (peek().type == WORD)
        {
            ASTNode* directive = parseDirective();
            location->addDirective(directive);
        }
        else
            throw std::runtime_error("Location: invalid token");
    }
    expect(RBRACE);
    return location;
}

ASTNode* ConfigParser::parseIndex()
{
    std::string path = expectWord();
    expect(SEMICOLON);
    return(new IndexNode(path));
}

ASTNode* ConfigParser::parseAllowedMethods()
{
    std::vector<std::string> methods;

    while (peek().type == WORD)
    {
        std::string method = expectWord();
        if (method == "GET" || method == "POST" || method == "DELETE")
            methods.push_back(method);
        else 
            throw std::runtime_error("Location: invalid method");
    }
    if (methods.empty())
        throw std::runtime_error("No methods provided");
    expect(SEMICOLON);
    return new AllowedMethodsNode(methods);
}
