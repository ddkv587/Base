#include "CBase.hpp"

#ifdef OS_QNX
	#include <malloc.h>
#endif

#if defined(OS_LINUX) || defined(OS_ANDROID)
	#include <malloc.h>
#endif

#ifdef OS_WIN
#endif

namespace Base
{
	void* Memory::malloc(size_t size, const CHAR* pcClassName, UINT uiClassID)
	{
		CMemManager* pMemManager = CMemManager::getInstance();

		if (pMemManager != NULL)
		{
			return pMemManager->malloc(size, pcClassName, uiClassID);
		}

		return ::malloc(size);
	}

	void Memory::free(void* p)
	{
		if (p == NULL)
		{
			return;
		}

		CMemManager* pMemManager = CMemManager::getInstance();

		if (pMemManager != NULL)
		{
			return pMemManager->free(p);
		}

		return ::free(p);
	}

	INT Memory::checkPtr(void* p, const CHAR* pcHint)
	{
		CMemManager* pMemManager = CMemManager::getInstance();

		if (pMemManager != NULL)
		{
			return pMemManager->checkPtr(p);
		}

		return -1;
	}

	UINT Memory::registClassName(const CHAR* pcClassName)
	{
		CMemManager* pMemManager = CMemManager::getInstance();

		if (pMemManager != NULL)
		{
			return pMemManager->registClassName(pcClassName);
		}

		return 0;
	}
}
