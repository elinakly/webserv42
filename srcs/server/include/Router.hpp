#pragma once
#include <map>
#include <string>
#include <iostream>
#include <sys/stat.h>

struct Server;
class LocationNode;

class Router
{
private:
	std::map<std::string, std::string> _mapPath;
	std::string	_statusCode;
	std::string	_mainPath;
	std::string	_statusReason;
	std::string _redirectPath;
	std::string _directoryListing;

public:
	Router() = default;
	~Router() = default;
    void setStatusCode(std::string statusCode){_statusCode = statusCode;};
	std::string	getStatusCode() {return(_statusCode);};
	std::string	getStatusReason() {return(_statusReason);};
	std::string getRedirectPath() {return(_redirectPath);};
	std::string &getDirectoryListing() {return(_directoryListing);};
    std::string Path(const std::string &root, const std::string &requestPath, std::string &index);
    
    const LocationNode* findBestLocation(const Server &server, const std::string & requestPath);
    std::string getErrorPagePath(Server *config, const std::string &statusCode);
	std::string getDefaultErrorPage(int code);
	std::string buildErrorResponsePath(Server *config, const std::string &statusCode);
	std::string routeRequest(const HTTPRequest& req, Server* config);
	std::string buildFilePath(const std::string &root, const std::string &requestPath, std::string &index, const LocationNode *location);
	std::string	generateDirectoryListing(const std::string &path, const std::string &url); 
};
