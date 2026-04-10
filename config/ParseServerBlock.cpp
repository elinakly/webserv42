#include "ConfigParser.hpp"

ServerNode* ConfigParser::parseServer()
{
    if (peek().type != WORD || peek().value != "server")
        throw std::runtime_error("Expected 'server'");
    advance();
    expect(LBRACE);
    ServerNode* server = new ServerNode();
    while(peek().type != RBRACE)
    {
        if(peek().type == WORD && peek().value == "location")
        {
            ASTNode* loc = parseLocation();
            server->addLocation(loc);
        }
        else if(peek().type == WORD)
        {
            ASTNode* directive = parseDirective();
            server->addDirective(directive);
        }
        else
        {
            throw std::runtime_error("Invalid token in server block");
        }
    }
    expect(RBRACE);
    return(server);
}

ASTNode* ConfigParser::parseDirective()
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
    throw std::runtime_error("Invalid directive in server block");
}

ASTNode*  ConfigParser::parseListen()
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
    return(new ListenNode(port));
}

ASTNode* ConfigParser::parseRoot()
{
    std::string path = expectWord();
    expect(SEMICOLON);
    return (new RootNode(path));
}

ASTNode*  ConfigParser::parseServerName()
{
    std::string name = expectWord();
    expect(SEMICOLON);
    return (new ServerNameNode(name));
}

ASTNode*  ConfigParser::parseErrorPage()
{
    std::string error = expectWord();
    ErrorPageNode* node = new ErrorPageNode();
    std::vector<int> codes;

    while (peek().type == WORD)
    {
        std::string word = peek().value;
        if (!std::isdigit(word[0]))
            break;
        for (size_t i = 0; i < word.size(); i++)
        {
            if (!std::isdigit(word[i]))
                throw std::runtime_error("error_page: invalid code");
        }
        int code = std::atoi(word.c_str());
        codes.push_back(code);
        advance();
    }
    if (codes.empty())
        throw std::runtime_error("error_page: no error codes");
    std::string path = expectWord();
    expect(SEMICOLON);
    for (size_t i = 0; i < codes.size(); i++)
    {
        node->addErrors(codes[i], path);
    }
    return node;
}

ASTNode*  ConfigParser::parseMaxBodySize()
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
    return(new MaxBodySizeNode(size));
}

void ServerNode::addDirective(ASTNode* directive)
{
    _directives.push_back(directive);
}

void ServerNode::addLocation(ASTNode* location)
{
    _locations.push_back(location);
}

