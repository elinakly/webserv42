#include "ConfigParser.hpp"

ConfigParser::~ConfigParser()
{
    clear();
}

void ConfigParser::parse(const std::string& file_name)
{
    std::ifstream file(file_name.c_str());
    if(!file)
        throw std::runtime_error("Faild to open file");
    clear();
    tokenize(file);
    _pos = 0;
    while(peek().type != END)
    {
        if(peek().value == "server" && peek().type == WORD)
            _servers.push_back(parseServer());
        else
            throw std::runtime_error("Unexpected token");
    }
    
}

const std::vector<std::unique_ptr<ServerNode> >& ConfigParser::getServers() const noexcept
{
    return(_servers);
}

void ConfigParser::tokenize(std::ifstream& file)
{
    char c;

    while(file.get(c))
    {
        if (std::isspace(static_cast<unsigned char>(c)))
            continue;

        if (c == '{')
        {
           _tokens.push_back({LBRACE, "{"});
           continue;
        }
        if (c == '}')
        {
           _tokens.push_back({RBRACE, "}"});
           continue;
        }
        if (c == ';')
        {
           _tokens.push_back({SEMICOLON, ";"});
           continue;
        }
        if(c == '#')
        {
            while (file.get(c) && c != '\n')
                ;
            continue;
        }
        std::string word;
        word += c;

        while (file.peek() != EOF)
        {
            char p = file.peek();
            if (std::isspace(static_cast<unsigned char>(p)) ||
                p == '{' || p == '}' || p == ';')
                break;
            file.get(c);
            word += c;
        }

        if (!word.empty())
            _tokens.push_back({WORD, word});
    }
    _tokens.push_back({END, ""});
    // for(size_t i = 0; i < _tokens.size(); i++)
    // {
    //     std::cout << _tokens[i];
    // }
}

std::ostream& operator<<(std::ostream& os, const Token& t)
{
    os << "[" << t.type << ", " << t.value << "]\n";
    return os;
}

std::string ConfigParser::expectWord()
{
    if(peek().type != WORD)
        throw std::runtime_error("Expected WORD token");
    std::string value = peek().value;
    advance();
    return(value);
}

void ConfigParser::expect(TokenType type)
{
    if (peek().type != type)
        throw std::runtime_error("Syntax error: unexpected token");
    advance();
}

const Token& ConfigParser::peek() const
{
    return _tokens[_pos];
} //look at current token 

Token ConfigParser::advance()
{
    if (_pos >= _tokens.size())
        throw std::runtime_error("Unexpected end of tokens");

    return _tokens[_pos++];
} //return current token AND move forward


void ConfigParser::clear() noexcept
{
    _pos = 0;

    _servers.clear();
    _tokens.clear();
}
