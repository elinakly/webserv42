#include "CgiHandler.hpp"

CgiHandler::CgiHandler(const HTTPRequest &request, const LocationNode &location) : _request(request), _location(location)
{
	this->_body = request.getBody();
	this->_uri = request.getUri();
	this->_method = request.getMethod();
	size_t tmpPath;
	std::string	extension;
	std::map<std::string, std::string> map;

	// Отделяем query string от пути
	std::string requestPath = request.getPath();
	size_t queryPos = requestPath.find("?");
	if (queryPos != std::string::npos)
		requestPath = requestPath.substr(0, queryPos);

	_path = location.getRoot() + requestPath;

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
	size_t	pos;
	if ((pos = _request.getPath().find("?")) != std::string::npos)
		query = _request.getPath().substr(pos + 1);
	_env["QUERY_STRING"] = query;
	_env["CONTENT_LENGTH"] = std::to_string(_body.size());
	_env["CONTENT_TYPE"] = "";
	std::string pathInfo = _request.getPath();

	size_t queryPos = pathInfo.find('?');
	if (queryPos != std::string::npos)
		pathInfo = pathInfo.substr(0, queryPos);
	_env["PATH_INFO"] = pathInfo;
	_env["SCRIPT_NAME"] = pathInfo;
	_env["SCRIPT_FILENAME"] = _path;
	_env["REQUEST_URI"] = _request.getPath();
	_env["SERVER_NAME"] = "localhost";
	_env["SERVER_PORT"] = "8080";	
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
std::string CgiHandler::executeScript()
{
	int	pipeIn[2];
	int	pipeOut[2];

	if (pipe(pipeIn) < 0 || pipe(pipeOut) < 0)
		throw(std::runtime_error("Pipe Failed"));
	pid_t	pid = fork();
	if (pid < 0)
	{
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);
		throw(std::runtime_error("Fork Failed"));
	}
	if (pid == 0)
	{
		close(pipeIn[1]);
		close(pipeOut[0]);
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[0]);
		close(pipeOut[1]);
		char	*args[3];
		args[0] = const_cast<char*>(_inter.c_str());
		args[1] = const_cast<char*>(_path.c_str());
		args[2] = NULL;
		execve(args[0], args, converEnvToChar());
		exit(1);
	}
	else
	{
		close(pipeIn[0]);
		close(pipeOut[1]);
		if (!_body.empty())
			write(pipeIn[1], _body.c_str(), _body.size());
		close(pipeIn[1]);
		int	status;
		time_t	start = time(NULL);
		while (true)
		{
			pid_t ret = waitpid(pid, &status, WNOHANG);
			if (ret == pid)
				break;
			if (ret == -1)
				throw(std::runtime_error("waitpid failed"));
			if (time(NULL) - start >= 5)
			{
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				throw(std::runtime_error("CGI timeout"));
			}
			usleep(1000);
		}
			
		std::string res;
		char	buff[1024];
		int	bytes;
		
		while ((bytes = read(pipeOut[0], buff, sizeof(buff) - 1)) > 0)
		{
			buff[bytes] = '\0';
			res += buff;
		}
		close(pipeOut[0]);
		return(res);
	}
}

std::string CgiHandler::execute()
{
	return (executeScript());
}
