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
	this->setEnv();
}
void	CgiHandler::setEnv()
{
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["SERVER_SOFTWARE"] = "webserv/1.0";
	_env["REQUEST_METHOD"] = _method;
	_env["PATH_TRANSLATED"] = _path;
	
	std::string query = "";
	int	pos;
	if ((pos = _request.getPath().find("?")) != std::string::npos)
		query = _request.getPath().substr(pos + 1);
	_env["QUERY_STRING"] = query;
	_env["CONTENT_LENGTH"] = std::to_string(_body.size());
	_env["CONTENT_TYPE"] = "";
}
char **CgiHandler::converEnvToChar()
{	
	char **env = new char *[_env.size() + 1];
	size_t i = 0;

	for (auto const &pair : _env)
	{
		std::string tmp = pair.first + "=" + pair.second;
		env[i] = strdup(tmp.c_str());
		i++;
	}
	env[i] = NULL;
	return(env);
}