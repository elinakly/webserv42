#include "ErrorPageNode.hpp"

void ErrorPageNode::addErrors(int code, std::string& path)
{
    _errors[code] = path;
}

std::string  ErrorPageNode::getPath(int code) noexcept
{
    auto it = _errors.find(code);
    if (it == _errors.end())
        return("");
    return(it->second);
}