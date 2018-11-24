/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif
#include <map>

namespace Library
{
	class ServiceContainer
	{
	public:
		ServiceContainer();

		void AddService(unsigned int typeID, void* service);
		void RemoveService(unsigned int typeID);
		void* GetService(unsigned int typeID) const;

	private:
		ServiceContainer(const ServiceContainer& rhs);
		ServiceContainer& operator=(const ServiceContainer& rhs);

		std::map<unsigned int, void*> mServices;
	};
}
