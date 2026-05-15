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

HTTPRequest::HTTPRequest() : _rawRequest("") {}

bool HTTPRequest::parse()
{
	std::istringstream lineReader(_rawRequest); // Assigning lineReader to _rawRequest
	std::string string;
	if (!std::getline(lineReader, string)) // Reading lineReader till the \n
		return(false);
	std::istringstream stringReader(string); 
	if (!(stringReader >> _method >> _path >> _version)) // Reading "GET" "/" "HTTP(version)"
		return(false);
	//checking methods
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
	{
		_status_reason = "405 Method Not Allowed";
		return(false);
	}
	while (std::getline(lineReader, string)) // reading whole header
	{
		//if end of the header, than header is completed
		if (string.empty() || string == "\r")
			break ;
		//getting a key-value 
		size_t pos = string.find(":");
		if (pos == std::string::npos)
			return (false);
		//getting rid of the spaces 
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
		//placing it in the map
		if (!key.empty())
			_headers[key] = value;
	}
	//Body parsing
	
    std::string	body;
	//reading everything that left in the header
	while (std::getline(lineReader, string))
	{
		body += string;
		if (!lineReader.eof())
			body += "\n";
	}
	//placing it in the body
	//not http safe get rid of getline in the future
	_body = body;
	std::map<std::string, std::string>::iterator it;
	//checking for the content length
	it = _headers.find("Content-Length");
	//if there body
	if (it != _headers.end())
	{
		int	expected = std::atoi(it->second.c_str());
		//checking the size of content length and comperes it
		if ((int)_body.size() != expected)
		//if content length is not equals to the size of the body than returns false
			return (false);
	}
	return(true);
}
