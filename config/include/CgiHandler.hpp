#pragma once

#include "HttpRequest.hpp"
#include "LocationNode.hpp"
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
class CgiHandler
{
	private:
		const	HTTPRequest &_request;

		std::string	_uri;
		std::string	_method;
		std::string _body;
		std::map<std::string, std::string> _env;
		std::string _path;
		std::string	_inter;
		pid_t _pid;
		int	_pipeOut;
		int	_pipeIn;

		void		setEnv();
		char		**converEnvToChar();

	public:
		CgiHandler(const HTTPRequest &request, const LocationNode &location);
		~CgiHandler();

		pid_t start();
		int	getPipeFd() const;
		pid_t getPid() const;
		void writeBody();
};
