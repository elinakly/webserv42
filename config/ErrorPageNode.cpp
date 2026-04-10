#include "ErrorPageNode.hpp"

const void ErrorPageNode::addErrors(int code, std::string& path)
{
    errors[code] = path;
}

std::string  ErrorPageNode::getPath(int code)
{
    auto it = errors.find(code);
    if (it == errors.end())
        return("");
    return(it->second);
}