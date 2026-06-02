#include "CgiHandler.hpp"

CgiHandler::CgiHandler(const HTTPRequest &request, const LocationNode &location) : _request(request), _location(location)
{
	this->_body = request.getBody();
	this->_path = request.getPath();
	std::cout << request.getUri();
}