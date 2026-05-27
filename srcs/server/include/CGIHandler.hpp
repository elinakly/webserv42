#include <string>
#include <map>
#include <HttpRequest.hpp>

class CGIHandler
{
	private:
		std::string	_scriptPath;
		HTTPRequest _request;
		std::map<std::string, std::string>_CGIEnv;
		void	prepareENV(char **&envp);
	public:
		CGIHandler();
		void setScriptPath(const std::string& path);
		void setRequest(const HTTPRequest& req);
		void setEnv(const std::map<std::string, std::string>& env);
		std::string execute();
};
