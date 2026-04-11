#include "ConfigParser.hpp"

std::unique_ptr<ServerNode> ConfigParser::parseServer()
{
    if (peek().type != WORD || peek().value != "server")
        throw std::runtime_error("Expected 'server'");
    advance();
    expect(LBRACE);
    auto server = std::make_unique<ServerNode>();
    while(peek().type != RBRACE)
    {
        if (peek().type == END)
            throw std::runtime_error("Unexpected end in server block");

        if(peek().type == WORD && peek().value == "location")
        {
            std::unique_ptr<ASTNode> loc = parseLocation();
            server->addLocation(std::move(loc));
        }
        else if(peek().type == WORD)
        {
            std::unique_ptr<ASTNode> directive = parseDirective();
            server->addDirective(std::move(directive));
        }
        else
        {
            throw std::runtime_error("Invalid token in server block");
        }
    }
    expect(RBRACE);
    return(server);
}

std::unique_ptr<ASTNode> ConfigParser::parseDirective()
{
    std::string word = expectWord();
    if (word == "listen")
        return parseListen();
    else if(word == "root")
        return parseRoot();
    else if (word == "server_name")
        return parseServerName();
    else if (word == "error_page")
        return parseErrorPage();
    else if (word == "client_max_body_size")
        return parseMaxBodySize();
    else if (word == "index")
        return parseIndex();
    else if (word == "allowed_methods")
        return parseAllowedMethods();    
    throw std::runtime_error("Invalid directive in block");
}

std::unique_ptr<ASTNode> ConfigParser::parseListen()
{
    std::string value = expectWord();

    for (size_t i = 0; i < value.size(); i++)
    {
        if (!std::isdigit(value[i]))
            throw std::runtime_error("listen: invalid port");
    }

    int port = std::atoi(value.c_str());

    if (port <= 0 || port > 65535)
        throw std::runtime_error("listen: port out of range");

    expect(SEMICOLON);
    return std::make_unique<ListenNode>(port);
}
 
std::unique_ptr<ASTNode> ConfigParser::parseRoot()
{
    std::string path = expectWord();
    expect(SEMICOLON);
    return std::make_unique<RootNode>(path);
}

std::unique_ptr<ASTNode> ConfigParser::parseServerName()
{
    std::string name = expectWord();
    expect(SEMICOLON);
    return(std::make_unique<ServerNameNode>(name));
}

std::unique_ptr<ASTNode> ConfigParser::parseErrorPage()
{
    auto node = std::make_unique<ErrorPageNode>();
    std::vector<int> codes;

    while (peek().type == WORD)
    {
        const std::string& word = peek().value;

        if (word.empty())
            throw std::runtime_error("error_page: empty token");
        if (!std::isdigit(static_cast<unsigned char>(word[0])))
            break;
        for (size_t i = 0; i < word.size(); i++)
        {
            if (!std::isdigit(static_cast<unsigned char>(word[i])))
                throw std::runtime_error("error_page: invalid code");
        }
        int code = std::atoi(word.c_str());
        if (code > 599 || code < 300)
            throw std::runtime_error("error_page: code is out of range");
        codes.push_back(code);
        advance();
    }
    if (codes.empty())
        throw std::runtime_error("error_page: no error codes");
    std::string path = expectWord();
    expect(SEMICOLON);
    for (size_t i = 0; i < codes.size(); i++)
        node->addErrors(codes[i], path);
    
    return (node);
}

std::unique_ptr<ASTNode> ConfigParser::parseMaxBodySize()
{
    std::string value = expectWord();

    for (size_t i = 0; i < value.size(); i++)
    {
        if (!std::isdigit(value[i]))
            throw std::runtime_error("max_body_size: invalid size");
    }

    int size = std::atoi(value.c_str());

    if (size <= 0)
        throw std::runtime_error("max_body_size: size out of range");

    expect(SEMICOLON);
    return(std::make_unique<MaxBodySizeNode>(size));
}
