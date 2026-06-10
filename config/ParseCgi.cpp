#include "ConfigParser.hpp"

//This function os called when parser finds "CGI"
std::unique_ptr<ASTNode> ConfigParser::parseCgi()
{
	//creating a node container
    auto cgiNode = std::make_unique<CgiNode>();
	//reading a string per peers, while we are not found ";"
    while (peek().type == WORD)
    {
		//waiting for the ".py"
        std::string extension = expectWord();
		//waiting for "/usr/bin/python3"
        std::string path = expectWord();
		//adding it to the node
        cgiNode->addCgi(extension, path);
    }
	//at the end theres always supposed to be ";"
    expect(SEMICOLON);
    return cgiNode;
}
