#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"
#include "Router.hpp"

#include <algorithm>

std::string Router::routeRequest(const HTTPRequest& req, Server* config)
{
    _statusCode = "200";
    _statusReason = "OK";
    _redirectPath.clear();
    std::string filePath;

    std::string requestPath = req.getPath();
    size_t queryPos = requestPath.find("?");
    if (queryPos != std::string::npos)
        requestPath = requestPath.substr(0, queryPos);

    const LocationNode* location = findBestLocation(*config, requestPath);
    const std::vector<std::string>* methodsToCheck = &config->methods;

    const std::string& root = location ? location->getRoot() : config->root_path;
    std::string index = location ? location->getIndexPath() : config->index;
    std::string relativePath = requestPath;

    if (location)
    {
        const std::string &locpath = location->getPath();
        if (relativePath.find(locpath) == 0)
        {
            relativePath.erase(0, locpath.length());
            if (relativePath.empty())
                relativePath = "/";
        }
    }

    if (location)
    {
        const std::vector<std::string>& locationMethods = location->getAllowedMethods();
        if (!locationMethods.empty()) {
            methodsToCheck = &locationMethods;
        }
    }
    if (std::find(methodsToCheck->begin(), methodsToCheck->end(), req.getMethod()) == methodsToCheck->end()) 
    {
        _statusCode = "405";
        _statusReason = "Method Not Allowed";
    }
    
    else if (req.getBody().size() > (size_t)config->max_body_size)
    {
        _statusCode = "413";
        _statusReason = "Payload Too Large";
    }
    else if (location && location->getIsRedir())
    {
        int code = location->getCode();
        switch (code)
        {
            case 301:
                _statusCode = "301";
                _statusReason = "Moved Permanently";
                break;
            case 302:
                _statusCode = "302";
                _statusReason = "Found";
                break;
            case 307:
                _statusCode = "307";
                _statusReason = "Temporary Redirected";
                break;
            case 308:
                _statusCode = "308";
                _statusReason = "Moved Permanently";
                break;
            default:
                _statusCode = "302";
                _statusReason = "Found";
                break;
        }
        _redirectPath = location->getNewPath();
        return "";
    }
    if (_statusCode == "200")
    {
        if (req.getMethod() == "POST")
            filePath = root + relativePath;
        else
            filePath = buildFilePath(root, relativePath, index);
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
    std::map<int, std::string>::const_iterator it = config->errors.find(code);
    if (it != config->errors.end())
    // custom page was found returning path  "errors/404_not_found"
        return it->second;
    // if there's not than returing the main page
    return "";
}

std::string Router::getDefaultErrorPage(int code)
{
    switch (code)
    {
        case 400: return "<html><body><h1>400 Bad Request</h1></body></html>";
        case 403: return "<html><body><h1>403 Forbidden</h1></body></html>";
        case 404: return "<html><body><h1>404 Not Found</h1></body></html>";
        case 405: return "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        case 408: return "<html><body><h1>408 Request Timeout</h1></body></html>";
        case 413: return "<html><body><h1>413 Payload Too Large</h1></body></html>";
        case 500: return "<html><body><h1>500 Internal Server Error</h1></body></html>";
        case 502: return "<html><body><h1>502 Bad Gateway</h1></body></html>";
        case 504: return "<html><body><h1>504 Gateway Timeout</h1></body></html>";
        default: return "<html><body><h1>500 Internal Server Error</h1></body></html>";
    }
}

std::string Router::buildErrorResponsePath(Server *config, const std::string &statusCode)
{
        // 1. Try to find a configured error_page for the status code.
    int code = std::stoi(statusCode);

    std::string errorPath = getErrorPagePath(config, statusCode);
    if (!errorPath.empty())
    {
        // 2. Build absolute path using server root and index fallback.
        std::string filePath = buildFilePath(config->root_path, errorPath, config->index);
        if (!filePath.empty())
        // 3. Return the resolved error file path for the response body.
            return filePath;
    }

    return getDefaultErrorPage(code);
}
const LocationNode* Router::findBestLocation(const Server &server, const std::string &requestPath)
{
    const LocationNode *bestLocation = nullptr;
    size_t len = 0;

    for (const LocationNode *locationPtr : server.locations)
    {
        const std::string &locPath = locationPtr->getPath();

        if (locPath == "/redirect" && requestPath != "/redirect")
            continue;

        if (requestPath.rfind(locPath, 0) == 0)
        {
            bool match = false;

            if (requestPath.size() == locPath.size())
                match = true;
            else if (locPath.back() == '/')
                match = true;
            else if (requestPath[locPath.size()] == '/')
                match = true;

            if (match && locPath.length() > len)
            {
                len = locPath.length();
                bestLocation = locationPtr;
            }
        }
    }
    return bestLocation;
}