#include "ServerMaster.hpp"
#include "RootNode.hpp"
#include "IndexNode.hpp"

std::string ServerMaster::findLocationRoot(const LocationNode * location, Server *config)
{
    std::string root = config->root_path;
    if (!location)
        return (root);
    for (const auto& directives : location->getDirectives())
    {
        if (const RootNode* node = dynamic_cast<const RootNode*>(directives.get()))
            return(node->getPath());
    }
    return(root);
}
std::string ServerMaster::findLocationIndex(const LocationNode *location, Server *config)
{
    std::string index = config->index;
    if (!location)
        return(index);
    for (const auto & directives : location->getDirectives())
    {
        if (const IndexNode *index = dynamic_cast<const IndexNode*>(directives.get()))
            return(index->getPath());
    }
    return(index);
}
std::string ServerMaster::buildFilePath(const std::string &root, const std::string &requestPath, std::string &index, std::string status)
{
    std::string filePath = root + requestPath;
    struct stat PathStats;
    if (stat(filePath.c_str(), &PathStats) != 0)
    {
        status = "404 Not Found";
        return (filePath);
    }
    if (S_ISDIR(PathStats.st_mode))
    {
        if (filePath.back() != '/')
            filePath += '/';
        filePath += index;
        if (stat(filePath.c_str(), &PathStats)!= 0)
            status = "404 Not Found";
    }
    return (filePath);
} 
const LocationNode*	findBestLocation(const Server &server, const std::string & requestPath)
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