/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "ServiceContainer.h"

namespace Library
{
	ServiceContainer::ServiceContainer()
		: mServices()
	{
	}

	void ServiceContainer::AddService(unsigned int typeID, void* service)
	{
		mServices.insert(std::pair<unsigned int, void*>(typeID, service));
	}

	void ServiceContainer::RemoveService(unsigned int typeID)
	{
		mServices.erase(typeID);
	}

	void* ServiceContainer::GetService(unsigned int typeID) const
	{
		return mServices.at(typeID);
	}
}
