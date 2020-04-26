#include "CLock.h"

CLock::CLock()
{
	::InitializeCriticalSection(&m_CriticalSection);
}

CLock::~CLock()
{
	::DeleteCriticalSection(&m_CriticalSection);
}

void CLock::Lock()
{
	::EnterCriticalSection(&m_CriticalSection);
}

void CLock::UnLock()
{
	::LeaveCriticalSection(&m_CriticalSection);
}
