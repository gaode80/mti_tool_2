#pragma once
/*******************************************************
author:       gaojy
create date:  2019-11-07
function:     Ëø
********************************************************/
#include <windows.h>

class CLock
{
public:
	CLock();
	~CLock();

	void Lock();
	void UnLock();

private:
	CRITICAL_SECTION m_CriticalSection;
};

