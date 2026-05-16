#include "ConfigParser.hpp"

std::unique_ptr<ASTNode> ConfigParser::parseLocation()
{
    std::string word = expectWord();
    if (word != "location")
        throw std::runtime_error("Expected 'location'");

    std::string path = expectWord();
    expect(LBRACE);
    auto location = std::make_unique<LocationNode>(path);

    while (peek().type != RBRACE)
    {
        if (peek().type == END)
            throw std::runtime_error("Unexpected end in location block");
        if (peek().type == WORD)
        {
            std::unique_ptr<ASTNode> directive = parseDirective();

            if (Return* ret = dynamic_cast<Return*>(directive.get()))
            {
                location->setRedir(true);
                location->setNewPath(ret->getNewPath());
            }

            location->addDirective(std::move(directive));
        }
        else
            throw std::runtime_error("Location: invalid token");
    }
    expect(RBRACE);
    return location;
}

std::unique_ptr<ASTNode> ConfigParser::parseIndex()
{
    std::string path = expectWord();
    expect(SEMICOLON);
    return(std::make_unique<IndexNode>(path));
}

std::unique_ptr<ASTNode> ConfigParser::parseAllowedMethods()
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
    return(std::make_unique<AllowedMethodsNode>(methods));
}

std::unique_ptr<ASTNode> ConfigParser::parseReturn()
{
    std::string newPath;
    while(peek().type == WORD)
    {
        newPath = expectWord();
    }
    if (newPath.empty())
        throw std::runtime_error("No redirect path provided");
    expect(SEMICOLON);
    
    return(std::make_unique<Return>(newPath));
}

void LocationNode::addDirective(std::unique_ptr<ASTNode> directive) noexcept
{
    _directives.push_back(std::move(directive));
}