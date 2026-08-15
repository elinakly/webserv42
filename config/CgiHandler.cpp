#include "CgiHandler.hpp"

void	CgiHandler::writeBody()
{
	if (!_body.empty())
		write(_pipeIn, _body.c_str(), _body.size());
	//it passes the body of the HTTP request to the CGI via stdin
	close(_pipeIn);
	_pipeIn = -1;
}
int	CgiHandler::getPipeFd() const
{
	return (_pipeOut);
}

pid_t CgiHandler::getPid() const
{
	return (_pid);
}

CgiHandler::CgiHandler(const HTTPRequest &request, const LocationNode &location) : _request(request), _pid(-1), _pipeOut(-1), _pipeIn(-1)
{
	this->_body = request.getBody();
	this->_uri = request.getUri();
	this->_method = request.getMethod();
	//Getting the info from the header
	size_t tmpPath;
	std::string	extension;
	std::map<std::string, std::string> map;

	std::string requestPath = request.getPath();
	size_t queryPos = requestPath.find("?");
	if (queryPos != std::string::npos)
		requestPath = requestPath.substr(0, queryPos);
	//Getting absolute path
	_path = location.getRoot() + requestPath;
	//Creating the path for the CGI
	tmpPath = _path.rfind(".");
	if (tmpPath == std::string::npos)
		throw std::runtime_error("CGI extension not found");
	extension = _path.substr(tmpPath);
	//determiniting the extension
	map = location.getCgi();
	auto it = map.find(extension);
	if (it == map.end())
		throw std::runtime_error("Unsuported CGI Extension");
	_inter = it->second;
	//setting the interpritator AKA /usr/bin/python3
	this->setEnv();
}
CgiHandler::~CgiHandler()
{
	if (_pipeIn != -1)
		close(_pipeIn);
	if (_pipeOut != -1)
		close(_pipeOut);
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

pid_t CgiHandler::start()
{
	int	PipeIn[2];
	int	PipeOut[2];

	//Creating 2 pipes one pipe goes from Servers to CGI the other one goes from  CGI to Server
	if (pipe(PipeIn) < 0 || pipe(PipeOut) < 0)
		throw std::runtime_error("Pipe Failed");
	_pipeIn = PipeIn[1];
	_pipeOut = PipeOut[0];
	_pid = fork();

	if (_pid < 0)
	{
		close(PipeIn[0]);
		close(PipeIn[1]);
		close(PipeOut[0]);
		close(PipeOut[1]);
		throw std::runtime_error("Fork Failed");
	}
	//Child proccess has to only read from the pipe
	if (_pid == 0)
	{
		close(PipeIn[1]);
		close(PipeOut[0]);
		dup2(PipeIn[0], STDIN_FILENO);
		dup2(PipeOut[1], STDOUT_FILENO);
		close(PipeIn[0]);
		close(PipeOut[1]);

		char *args[3];
		args[0] = const_cast<char*>(_inter.c_str());
		args[1] = const_cast<char*>(_path.c_str());
		args[2] = NULL;
		execve(args[0], args, converEnvToChar());
		//Setting the execve to redirect to the path to run the CGI
		exit(1);
	}
	close(PipeIn[0]);
	close(PipeOut[1]);
	fcntl(_pipeOut, F_SETFL, O_NONBLOCK);
	//it switches the pipe from reading from CGI to non blocking mode
	return(_pid);
}