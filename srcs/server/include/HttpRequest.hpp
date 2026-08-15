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
        std::string _uri;
		std::map<std::string, std::string>_headers;
	public:
		void	setStatus(std::string status) {_status_reason = status;}
        const std::string	getMethod() const {return(_method);};
        const std::string	getPath() const;
        const std::string 	getStatusReason() const;
        const std::string	getVersion() const;
        const std::string	getBody() const {return(_body);};
        const std::string	getRequest() const {return(_req);};
        const std::string getUri()const {return(_uri);};
        HTTPRequest() ;
		~HTTPRequest() = default;
		HTTPRequest(std::string &string);
    
        bool parse();
};