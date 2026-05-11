#include "ServerMaster.hpp"

bool	ServerMaster::routing()
{
	std::string mathingServers;
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

}