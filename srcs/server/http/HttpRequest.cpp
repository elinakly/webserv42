#include "HttpRequest.hpp"

#include <sstream>
#include <iostream> 
const std::string HTTPRequest::getVersion() const
{
    return(_version);
}
HTTPRequest::HTTPRequest(std::string &string)
{
    _rawRequest = string;
}
const std::string HTTPRequest::getStatusReason() const
{
    return(_status_reason);
}

const std::string HTTPRequest::getPath() const
{
    return(_path);
}
// static std::string sanitizeTargetToPath(const std::string& target)
// {
//     if (target.empty() || target[0] != '/')
//         return "";

//     std::string cleanTarget = target;

//     size_t queryPos = cleanTarget.find('?');
//     if (queryPos != std::string::npos)
//         cleanTarget.erase(queryPos);

//     if (cleanTarget.find("..") != std::string::npos)
//         return "";

//     if (cleanTarget == "/")
//         return "./www/index.html";

//     if (cleanTarget == "/")
//         return "./www/index.html";
//     if (cleanTarget == "/errors/404")
//         return "./www/errors/404_not_found.html";

//     return "./www" + cleanTarget;
// }

HTTPRequest::HTTPRequest() : _rawRequest("") {}

bool HTTPRequest::parse()
{
	std::istringstream lineReader(_rawRequest); // Assigning lineReader to _rawRequest
	std::string string;
	if (!std::getline(lineReader, string)) // Reading lineReader till the \n
		return(false);
	std::istringstream stringReader(string); 
	if (! (stringReader >> _method >> _path >> _version)) // Reading "GET" "/" "HTTP(version)"
		return(false);
	while (std::getline(lineReader, string)) // reading whole header
	{
		if (string.empty() || string == "\r")
			break ;
		size_t pos = string.find(":");
		std::string key = string.substr(0, pos);
		std::string value = string.substr(pos + 1);
		size_t	FkeyTrim = key.find_first_not_of(" \t\n\r");
		size_t	LkeyTrim = key.find_last_not_of(" \t\n\r");
		size_t	FvalTrim = value.find_first_not_of(" \t\n\r");
		size_t	LvalTrim = value.find_last_not_of(" \t\n\r");
		if (FkeyTrim != std::string::npos)
			key = key.substr(FkeyTrim, (LkeyTrim - FkeyTrim + 1));
		else
			key = "";
		if (FvalTrim != std::string::npos)
			value = value.substr(FvalTrim, (LvalTrim - FvalTrim + 1));
		else
			value = "";
		if (!key.empty())
			_headers[key] = value;
	}
    
	return(true);
}
