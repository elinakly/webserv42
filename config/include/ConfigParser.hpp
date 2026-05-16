#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <memory>

#include "ASTNode.hpp"
#include "ServerNode.hpp"
#include "LocationNode.hpp"
#include "ListenNode.hpp"
#include "ErrorPageNode.hpp"
#include "RootNode.hpp"
#include "MaxBodySizeNode.hpp"
#include "ServerNameNode.hpp"
#include "IndexNode.hpp"
#include "AllowedMethodsNode.hpp"
#include "HostNode.hpp"
#include "Return.hpp"

enum TokenType{
    WORD,
    LBRACE,
    RBRACE,
    SEMICOLON,
    END
};

struct Token{
    TokenType type;
    std::string value;
};

class ConfigParser {
    private:
        size_t _pos;
        std::vector<std::unique_ptr<ServerNode>> _servers;
        std::vector<Token> _tokens;


    public:
        ConfigParser() = default;
        ~ConfigParser();

        void parse(const std::string& file_name);
        const std::vector<std::unique_ptr<ServerNode> >& getServers() const noexcept;
    
    private:
        void tokenize(std::ifstream& file);
        std::string expectWord();
        void expect(TokenType type);
        const Token& peek() const; //look at current token 
        Token advance(); //return current token AND move forward

        std::unique_ptr<ServerNode> parseServer();
        
        std::unique_ptr<ASTNode> parseDirective();
        std::unique_ptr<ASTNode> parseLocation();
        std::unique_ptr<ASTNode> parseListen();
        std::unique_ptr<ASTNode> parseRoot();
        std::unique_ptr<ASTNode> parseServerName();
        std::unique_ptr<ASTNode> parseErrorPage();
        std::unique_ptr<ASTNode> parseHost();
        std::unique_ptr<ASTNode> parseMaxBodySize();
        std::unique_ptr<ASTNode> parseIndex();
        std::unique_ptr<ASTNode> parseAllowedMethods();
        std::unique_ptr<ASTNode> parseReturn();

        void clear() noexcept;
};

std::ostream& operator<<(std::ostream& os, const Token& t);