#include "CgiHandler.hpp"

CgiHandler::CgiHandler(const HTTPRequest &request, const LocationNode &location) : _request(request), _location(location)
{
	this->_body = request.getBody();
	this->_uri = request.getUri();
	this->_method = request.getMethod();
	int tmpPath;
	std::string	extension;
	std::map<std::string, std::string> map;

	_path = location.getRoot() + request.getPath();

	tmpPath = _path.rfind(".");
	if (tmpPath == std::string::npos)
		throw std::runtime_error("CGI extension not found");
	extension = _path.substr(tmpPath);
	map = location.getCgi();
	auto it = map.find(extension);
	if (it == map.end())
		throw std::runtime_error("Unsuported CGI Extension");
	_inter = it->second;

}