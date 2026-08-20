#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"
#include "Router.hpp"
#include <dirent.h>
#include <sstream>

#include <algorithm>

std::string Router::routeRequest(const HTTPRequest& req, Server* config)
{
    _statusCode = "200";
    _statusReason = "OK";
    _redirectPath.clear();
    std::string filePath;
    //Every new request assuming that its 200

    std::string requestPath = req.getPath();
    //getting path from the HTTP request
    if (!requestPath.empty() && requestPath[0] != '/')
        requestPath = "/" + requestPath;
    size_t queryPos = requestPath.find("?");
    if (queryPos != std::string::npos)
        requestPath = requestPath.substr(0, queryPos);
    //removing the query string

    const LocationNode* location = findBestLocation(*config, requestPath);
    const std::vector<std::string>* methodsToCheck = &config->methods;

    const std::string& root = location ? location->getRoot() : config->root_path;
    std::string index = location ? location->getIndexPath() : config->index;
    std::string relativePath = requestPath;
    if (location)
    {
        const std::string &locpath = location->getPath();

        if (locpath != "/" && relativePath.find(locpath) == 0)
        {
            relativePath.erase(0, locpath.length());

            if (relativePath.empty())
                relativePath = "/";
        }
    }
    //this path removes "location" path from the begining of the URL
    if (location)
    {
        const std::vector<std::string>& locationMethods = location->getAllowedMethods();
        if (!locationMethods.empty())
            methodsToCheck = &locationMethods;
        //But if a specific location has its own methods then we're using its method
    }
    if (std::find(methodsToCheck->begin(), methodsToCheck->end(), req.getMethod()) == methodsToCheck->end()) 
    {
        _statusCode = "405";
        _statusReason = "Method Not Allowed";
    }
    // Checking the HTTP methods
    else if (req.getBody().size() > (size_t)config->max_body_size)
    {
        _statusCode = "413";
        _statusReason = "Payload Too Large";
    }
    //Cheching the size
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
    //if the location redirect then we're checking the status code of the redirect
    if (_statusCode == "200")
    {
        if (req.getMethod() == "POST")
        {
            std::string upload = requestPath;
            if (location)
            {
                const std::string &locPath = location->getPath();
                if (locPath != "/" && upload.find(locPath) == 0)
                {
                    upload.erase(0, locPath.length());
                    if (upload.empty())
                        upload = "/";
                }
            }
            filePath = root + upload;
        //if its post then we're creating the path to get the post
        }
        else
        {
            filePath = buildFilePath(root, requestPath, index, location);
        }
        //if its GET then we're checking the file of itself
    }
    return filePath;
}
std::string Router::buildFilePath(const std::string &root, const std::string &requestPath, std::string &index, const LocationNode *location)
{
    std::string filePath = root + requestPath;
    struct stat pathStats;
    //getting and creating the path
    if (stat(filePath.c_str(), &pathStats) != 0)
    {
        _statusCode = "404";
        _statusReason= "Not Found";
        return "";
    }
    //if file does not exist then its 404
    if (S_ISDIR(pathStats.st_mode))
    {
        std::string dirPath = filePath;
        if (filePath.back() != '/')
            filePath += '/';
        filePath += index;
        if (stat(filePath.c_str(), &pathStats) != 0)
        {
            if (location && location->getAutoIndex())
                return(dirPath);
            _statusCode = "404";
            _statusReason = "Not Found";
            return "";
        }
    }
    //if its a directory, and if its index does not exists but autoindex is on then directory itself is returned
    if (!(pathStats.st_mode & S_IROTH)) 
    {
        _statusCode = "403";
        _statusReason = "Forbiden";
        return "";
    }
    //Checking the permisions
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
    // try to find a configured error page for the status code.
    int code = std::stoi(statusCode);

    std::string errorPath = getErrorPagePath(config, statusCode);
    if (!errorPath.empty())
    {
        // build absolute path using server root and index fallback.
        std::string filePath = buildFilePath(config->root_path, errorPath, config->index, NULL);
        if (!filePath.empty())
            return filePath;
        // return the resolved error file path for the response body.
    }

    return getDefaultErrorPage(code);
}
const LocationNode* Router::findBestLocation(const Server &server, const std::string &requestPath)
{
    const LocationNode *bestLocation = nullptr;
    size_t len = 0;
    //it searches all of the locations to get the right one
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
std::string Router::generateDirectoryListing(const std::string &path, const std::string &url)
{
    DIR *dir = opendir(path.c_str());
    //opens the directory from the given path
    if (!dir)
        return("");
    //if directory cannot be opened then returns empty page
    std::stringstream html;
    //creates page for the HTML
    html << "<html><body>";
    html << "<h1>Index of " << url << "</h1>";
    html << "<ul>";
    //this creates the begining of the HTML page
    struct dirent *entry;
    //readdir returns the paths of the pages
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        html << "<li><a href=\"";
        //creates a link
        if (url.back() != '/')
            html << url << "/";
        else
            html << url;
        //this checks if theres a "/" at the end and if theres not then adds it
        html << name << "\">";
        html << name;
        html << "</a></li>";
    }
    html << "</ul></body></html>";
    //closes the HTML page
    closedir(dir);
    return(html.str());
}