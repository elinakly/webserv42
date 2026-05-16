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
		std::string _status_reason;
		std::map<std::string, std::string>_headers;
	public:
		// void	setMethod(std::string method);
		// void	setPath(std::string path);
		// void	setVersion(std::string version);
		// void	setBody(std::string body);
		// void	setRawRequest(std::string rawRequest);
		// void	setRequest(std::string request);
		// void	setHeaders(std::map<std::string, std::string>headers);

		std::string	getMethod() {return(_method);};
		const std::string	getPath() const;
		const std::string 	getStatusReason() const;
		const std::string	getVersion() const;
		const std::string	getBody() const {return(_body);};
		// std::string	getRawRequest();
		const std::string	getRequest() const {return(_req);};
		// std::map<std::string, std::string> getHeaders();

		HTTPRequest() ;
		~HTTPRequest() = default;
		HTTPRequest(std::string &string);

		bool	parse();
};