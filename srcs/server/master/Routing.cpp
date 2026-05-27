#include "HttpResponse.hpp"
#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"
#include "Router.hpp"
#include <algorithm>

std::string Router::respond(const HTTPRequest& req, Server* config, bool parseOk) {
    HTTPResponse response;
    std::string filePath;
    std::string statusCode, statusReason, extraHeaders, statusLine;
    if (!parseOk) 
    {
        statusCode = "400";
        statusReason = "Bad Request";
        filePath = buildErrorResponsePath(config, statusCode);
        statusLine = statusCode + " " + statusReason;
        return response.build(req, filePath, statusLine);
    }
    filePath = routeRequest(req, config);
    statusCode = getStatusCode();
    statusReason = getStatusReason();
    if (statusCode != "200" && statusCode != "204")
        filePath = buildErrorResponsePath(config, statusCode);
    if (statusCode == "302" && !getRedirectPath().empty())
        extraHeaders = "Location: " + getRedirectPath() + "\r\n";
    statusLine = statusCode;
    if (!statusReason.empty())
        statusLine += " " + statusReason;
    if (statusCode == "204") 
    {
        response.setVersion(req.getVersion());
        response.setStatusReason(statusLine) ;
        std::string resp204 = response.buildStatusLine();
        resp204 += "Content-Length: 0\r\nConnection: close\r\n\r\n";
        return resp204;
    } 
    else
        return response.build(req, filePath, statusLine, extraHeaders);
}

std::string Router::routeRequest(const HTTPRequest& req, Server* config)
{
    _statusCode = "200";
    _statusReason = "OK";
    _redirectPath.clear();
    std::string filePath;
    std::string root;
    std::string index;

    // 1. ИСПРАВЛЕНО: Проверка разрешенных методов из конфигурации
    const LocationNode* location = findBestLocation(*config, req.getPath());
    const std::vector<std::string>* methodsToCheck = &config->methods; // По умолчанию используем глобальные методы сервера
    if (location) {
        root = location->getRoot();
        index = location->getIndexPath();
    } else {
        root = config->root_path;
        index = config->index;
    }
    if (location) {
        const std::vector<std::string>& locationMethods = location->getAllowedMethods();
        if (!locationMethods.empty()) {
            methodsToCheck = &locationMethods; // Если у location есть свои методы, используем их
        }
    }
    // Проверяем, есть ли метод запроса в выбранном списке
    if (std::find(methodsToCheck->begin(), methodsToCheck->end(), req.getMethod()) == methodsToCheck->end()) 
    {
        _statusCode = "405";
        _statusReason = "Method Not Allowed";
    }
    // 2. Проверка размера тела
    else if (req.getBody().size() > (size_t)config->max_body_size)
    {
        _statusCode = "413";
        _statusReason = "Payload Too Large";
    }
    // 3. Редирект из location (return)
    else if (location && location->getIsRedir())
    {
        _statusCode = "302";
        _statusReason = "Found";
        _redirectPath = location->getNewPath();
        return "";
    }
    // DELETE
    if (_statusCode == "200" && req.getMethod() == "DELETE") {
        filePath = buildFilePath(root, req.getPath(), index);
        if (filePath.empty()) {
            _statusCode = "404";
            _statusReason = "Not Found";
            return "";
        }
        // Попытка удалить файл
        if (remove(filePath.c_str()) == 0) {
            _statusCode = "204";
            _statusReason = "No Content";
            return "";
        } else {
            _statusCode = "500";
            _statusReason = "Internal Server Error";
            return "";
        }
    }
    // GET/POST и др.
    if (_statusCode == "200") {
        filePath = buildFilePath(root, req.getPath(), index);
    }
    return filePath;
}
std::string Router::buildFilePath(const std::string &root, const std::string &requestPath, std::string &index)
{
    std::string filePath = root + requestPath;
    struct stat pathStats;
    // 1. Проверяем, существует ли путь
    if (stat(filePath.c_str(), &pathStats) != 0)
    {
        _statusCode = "404";
        _statusReason= "Not Found";
        return ""; // ИСПРАВЛЕНО: Возвращаем пустую строку при ошибке
    }
    // 2. Если это директория, строим путь к index-файлу
    if (S_ISDIR(pathStats.st_mode))
    {
        if (filePath.back() != '/')
            filePath += '/';
        filePath += index;
        // И снова проверяем, теперь уже для index-файла
        if (stat(filePath.c_str(), &pathStats) != 0)
        {
            _statusCode = "404"; // index-файл не найден
            _statusReason = "Not Found";
            return "";
        }
    }
    // 3. Проверяем права на чтение для ЛЮБОГО конечного файла
    if (!(pathStats.st_mode & S_IROTH)) 
    {
        _statusCode = "403"; // Доступ запрещен
        _statusReason = "Forbiden";
        return "";
    }
    // Если все проверки пройдены, статус остается "200"
    return filePath;
}
std::string Router::getErrorPagePath(Server *config, const std::string &statusCode)
{
    int code = std::stoi(statusCode);
    std::map<int, std::string>::const_iterator it = config->errors.find(code); // converting string into numbers
    if (it != config->errors.end())
    // custom page was found returning path  "errors/404_not_found"
        return (it->second);
    else
    // if there's not than returing the main page
        return ("");
}
std::string Router::buildErrorResponsePath(Server *config, const std::string &statusCode)
{
    // 1. Try to find a configured error_page for the status code.
    std::string errorPath = getErrorPagePath(config, statusCode);
    if (errorPath.empty())
        return "";
    // 2. Build absolute path using server root and index fallback.
    std::string filePath = buildFilePath(config->root_path, errorPath, config->index);
    if (filePath.empty())
        return "";
    // 3. Return the resolved error file path for the response body.
    return filePath;
}
const LocationNode*	Router::findBestLocation(const Server &server, const std::string & requestPath) 
{
	const LocationNode	*bestLocation = nullptr;
	size_t			len = 0;

	for (const LocationNode* locationPtr : server.locations)
	{
		const	std::string &locPath = locationPtr->getPath();
		//Checking for the prefix
		if (requestPath.rfind(locPath, 0) == 0)
			if (locPath.length() > len)
			{
				len = locPath.length();
				bestLocation = locationPtr;
			}
	}
	return bestLocation;
}