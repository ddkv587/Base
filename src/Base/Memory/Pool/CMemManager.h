/**
* @attention Copyright
* Copyright - All Rights Reserved
* HARMAN Shanghai - HMI Team of Software
* Email: Feng.Wang@harman.com
*/
#ifndef __BASE_CMEMMANAGER_H_INCLUDED__
#define __BASE_CMEMMANAGER_H_INCLUDED__

namespace BASE
{
	class CMemManager
	{
	public:// const define

	private:// const define

	public:// embed class or struct or enum
		class IMemListener
		{
		public:
			virtual void	onOutOfMemory(UINT uiSize) = 0;
			virtual void	onMemoryError() = 0;
		};

	private:// embed class or struct or enum

	public:// method
		static CMemManager*	getInstance();

		// inner method
		CMemManager();
		virtual ~CMemManager();

		void		setListener(IMemListener* pListener);

		void*		malloc(size_t size, const CHAR* pcClassName = NULL, UINT uiClassID = 0);
		void		free(void* p);
		INT			checkPtr(void* p, const CHAR* pcHint = NULL);

		UINT		registClassName(const CHAR* pcClassName);
		BOOLEAN		outputState(UINT uiGPUMemorySize = 0);

		BOOLEAN		hasMemChecker()							{ return ( NULL != m_pMemChecker ); }

		// =============== get total alloc state =================
		UINT		getCurrentAllocCount()					{ return NULL != m_pMemChecker ? m_pMemChecker->getCurrentAllocCount() : 0; }
		size_t		getCurrentAllocSize()					{ return NULL != m_pMemChecker ? m_pMemChecker->getCurrentAllocSize() : 0; }

		// =============== get alloc state by thread =================
		UINT		getThreadCount()						{ return NULL != m_pMemChecker ? m_pMemChecker->getThreadCount() : 0; }
		UINT		getThreadID( UINT uiIndex )				{ return NULL != m_pMemChecker ? m_pMemChecker->getThreadID( uiIndex ) : 0; }
		size_t		getThreadSize( UINT uiIndex )			{ return NULL != m_pMemChecker ? m_pMemChecker->getThreadSize( uiIndex ) : 0; }

		BOOLEAN		generatePoolConfig(const String& strFileName, UINT uiIncBytes);

        void		registThreadName( UINT tID, const String& strName );

	protected:// method
		CMemManager(const CMemManager&) {}
		CMemManager& operator =(const CMemManager&){return *this;}

	private:// method
		void		initialize();

	protected:// property

	private:// property
		IMemListener*	        m_pListener;

		CMemAllocator*	        m_pMemAllocator;
		CMemChecker*	        m_pMemChecker;

		CSyncObject		        m_syncCallback;
		BOOLEAN			        m_bCallbacking;
	};
}

#endif