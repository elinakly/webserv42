#pragma once

#include "ASTNode.hpp"
#include <string>
#include <map>

class CgiNode : public ASTNode {
public:
    typedef std::map<std::string, std::string> cgi_map_type;

private:
    cgi_map_type _cgi; //this is for saving the path for example for .py "usr/bin/python3"

public:
    CgiNode();
    ~CgiNode();

	//Method for adding new information to the map
    void addCgi(const std::string& extension, const std::string& path);
    const cgi_map_type& getCgi() const;
    void resolve(const ASTNode& server);
};