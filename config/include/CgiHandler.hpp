#pragma once

#include "HttpRequest.hpp"
#include "LocationNode.hpp"
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <string.h>


class CgiHandler
{
	private:
		const	LocationNode &_location;
		const	HTTPRequest &_request;

		std::string	_uri;
		std::string	_method;
		std::string _body;
		std::map<std::string, std::string> _env;
		std::string _path;
		std::string	_inter;

		void		setEnv();
		std::string	executeScript();
		char		**converEnvToChar();

	public:
		CgiHandler(const HTTPRequest &request, const LocationNode &location);
		~CgiHandler() = default;
		
		std::string execute();
};
