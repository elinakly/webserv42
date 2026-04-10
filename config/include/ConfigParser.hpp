#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

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
        std::vector<ServerNode*> _servers;
        std::vector<Token> _tokens;


    public:
       ConfigParser();
       ~ConfigParser();

       void parse(const std::string& file_name);
       const std::vector<ServerNode*>& getServers() const;
    
    private:
        void tokenize(std::ifstream& file);
        std::string expectWord();
        void expect(TokenType type);
        const Token& peek() const; //look at current token 
        Token advance(); //return current token AND move forward

        ServerNode* parseServer();
        
        ASTNode* parseDirective();
        ASTNode* parseLocation();
        ASTNode* parseListen();
        ASTNode* parseRoot();
        ASTNode* parseServerName();
        ASTNode* parseErrorPage();
        ASTNode* parseMaxBodySize();
        ASTNode* parseIndex();
        ASTNode* parseAllowedMethods();

        void clear();
};
