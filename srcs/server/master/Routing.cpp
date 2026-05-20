#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"
#include "Router.hpp"


std::string Router::routeRequest(const HTTPRequest& req, Server* config)
{
    _statusCode = "200";
    _statusReason = "OK";
    std::string filePath;

    // 1. ИСПРАВЛЕНО: Проверка разрешенных методов из конфигурации
    const LocationNode* location = findBestLocation(*config, req.getPath());
    const std::vector<std::string>* methodsToCheck = &config->methods; // По умолчанию используем глобальные методы сервера

    if (location) {
        const std::vector<std::string>& locationMethods = location->getAllowedMethods();
        if (!locationMethods.empty()) {
            methodsToCheck = &locationMethods; // Если у location есть свои методы, используем их
        }
    }

    // Проверяем, есть ли метод запроса в выбранном списке
    if (std::find(methodsToCheck->begin(), methodsToCheck->end(), req.getMethod()) == methodsToCheck->end()) {
        _statusCode = "405";
        _statusReason = "Method Not Allowed";
    }
    
    // 2. Проверка размера тела
    else if (req.getBody().size() > (size_t)config->max_body_size)
    {
        _statusCode = "413";
        _statusReason = "Payload Too Large";
    }

    // ... остальная часть функции ...
    // 3. Построение пути
    if (_statusCode == "200")
    {
        filePath = buildFilePath(config->root_path, req.getPath(), config->index);
    }
    // ...

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