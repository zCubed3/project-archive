#pragma once

#include <common.hpp>

#ifdef WINDOWS
#include <Windows.h>
#endif

class AgnosticThread 
{

public:

	static void ThreadSleep(int time)
	{

#ifdef WINDOWS
		Sleep(time);
#endif

	};

};