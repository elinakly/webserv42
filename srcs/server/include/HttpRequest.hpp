#pragma once

#include <string>
#include <map>

class HTTPRequest {
	private:
		std::string	_method;
		std::string	_path;
		std::string	_version;
		std::string	_body;
		std::string	_rawRequest;
		std::string	_req;
		std::map<std::string, std::string>_headers;
	public:
		// void	setMethod(std::string method);
		// void	setPath(std::string path);
		// void	setVersion(std::string version);
		// void	setBody(std::string body);
		// void	setRawRequest(std::string rawRequest);
		// void	setRequest(std::string request);
		// void	setHeaders(std::map<std::string, std::string>headers);

		// std::string	getMethod();
		std::string	getPath();
		// std::string	getVersion();
		// std::string	getBody();
		// std::string	getRawRequest();
		// std::string	getRequest();
		// std::map<std::string, std::string> getHeaders();

		HTTPRequest();
		~HTTPRequest();
		HTTPRequest(std::string &other); 

		bool	parse();
};