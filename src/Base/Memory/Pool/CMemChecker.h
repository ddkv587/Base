/**
* @attention Copyright
* Copyright - All Rights Reserved
* HARMAN Shanghai - HMI Team of Software
* Email: Feng.Wang@harman.com
*/

#ifndef __BASE_CMEMCHECKER_H_INCLUDED__
#define __BASE_CMEMCHECKER_H_INCLUDED__

namespace BASE
{
	class CMemChecker
	{
	public:// const define

	private:// const define

		static const INT SIZE_INFO_MAX_COUNT = 151;

	public:// embed class or struct or enum

	private:// embed class or struct or enum

		enum EBlockStatus
		{
			bsOk			= 0,
			bsFreed			= 1,
			bsHeaderError	= 2,
			bsTailError		= 3,
		};

		enum ELinkStatus
		{
			lsOk			= 0,
			lsHasBlockError	= 1,
			lsLinkCrashed	= 2
		};

		struct tagBlockHeader
		{
			UINT			uiMagic1;
			tagBlockHeader*	pNext;
			tagBlockHeader*	pPrev;
			UINT			uiSize;			// Exact size requested by user.
			UINT			uiClassID;		// Key of Class name, zero means unknown.
			UINT			uiThreadID;		// thread id for check
			UINT			uiMagic2;
		};
		typedef tagBlockHeader* PBlockHeader;

		struct tagBlockStat
		{
			size_t			uiBeginSize;
			size_t			uiEndSize;
			LLONG			llAllocTimes;
			UINT			uiCurrentCount;
			size_t			uiCurrentSize;
			UINT			uiPeakCount;
			size_t			uiPeakSize;

			tagBlockStat()
				: uiBeginSize(0)
				, uiEndSize(0)
				, llAllocTimes(0)
				, uiCurrentCount(0)
				, uiCurrentSize(0)
				, uiPeakCount(0)
				, uiPeakSize(0)
			{
				;
			}
		};

		struct tagThreadSize
		{
			UINT		tID;
			size_t		tSize;
		};

		struct tagClassStat
		{
			UINT			uiInstanceCount;
			size_t			uiTotalSize;

			tagClassStat()
				: uiInstanceCount(0)
				, uiTotalSize(0)
			{
				;
			}
		};
		typedef Map<String, tagClassStat, tagStringCompare>		MapClassStat;
		typedef Map<UINT, MapClassStat, tagNumCompare<UINT> >	MapThreadStat;

		class CAutoLocker
		{
		public:
			CAutoLocker(CMemChecker *pMemChecker, BOOLEAN bCtrlHook)
				: m_pMemChecker(pMemChecker)
				, m_bCtrlHook(bCtrlHook)
			{
				m_pMemChecker->lock(m_bCtrlHook);
			}

			virtual ~CAutoLocker()
			{
				m_pMemChecker->unlock(m_bCtrlHook);
			}

		private:
			CMemChecker*	m_pMemChecker;
			BOOLEAN			m_bCtrlHook;
		};

		class CLogger
		{
		public:
			CLogger(CFile* pFile)
				: m_pFile(pFile)
				, m_bHasError(FALSE)
				, m_uiLineCount(0)
			{
			}

			BOOLEAN writeLine(const CHAR* pcData, INT iStrLen = -1)
			{
				if (pcData == NULL)
				{
					return TRUE;
				}

				if (iStrLen < 0)
				{
					iStrLen = String::strlen(pcData);
				}
				if (iStrLen == 0)
				{
					return TRUE;
				}

				++m_uiLineCount;

				if (m_pFile == NULL)
				{
					String::printf(pcData);
					return TRUE;
				}

				UINT uiDataLen = iStrLen * 2;
				BYTE* pbtData = new BYTE[uiDataLen];
				String::convertToBYTE_UTF16(pcData, pbtData, uiDataLen);
				BOOLEAN bWriteRet = m_pFile->writeData(pbtData, uiDataLen);
				delete[] pbtData;

				if (!bWriteRet)
				{
					m_bHasError = TRUE;
					return FALSE;
				}
				return TRUE;
			}

			void writeLine(const String& strData)
			{
				writeLine((const CHAR*)strData, (INT)strData.length());
			}

			BOOLEAN hasError()
			{
				return m_bHasError;
			}

			UINT lineCount()
			{
				return m_uiLineCount;
			}

		private:
			CFile*	m_pFile;
			BOOLEAN	m_bHasError;
			UINT	m_uiLineCount;
		};

	public:// method
		static UINT		getBlockExtSize();

		CMemChecker() ;
		virtual ~CMemChecker();

		void			initialize(BOOLEAN bCompactSizeRange, CMemAllocator* pMemAllocator = NULL);

		void*			malloc(size_t size, UINT uiClassID = 0);
		void			free(void* p);
		INT				checkPtr(void* p, const CHAR* pcHint = NULL);

		UINT			registClassName(const CHAR* pcClassName);
        void			registThreadName( UINT uiThreadID, const String& strThreadName);

		void			setReportFile(const String& strReportFile);
		BOOLEAN			outputState(UINT uiGPUMemorySize = 0);

		// =============== get total alloc state =================
		size_t			getCurrentAllocSize()						{ return m_blockStatAll.uiCurrentSize; }
		UINT			getCurrentAllocCount()						{ return m_blockStatAll.uiCurrentCount; }

		// =============== get alloc state by thread =================
		UINT			getThreadCount();
		UINT			getThreadID( UINT uiIndex );
		size_t			getThreadSize( UINT uiIndex );

		BOOLEAN			generatePoolConfig(const String& strFileName, UINT uiIncBytes);

	protected:

	private: // method
		void			lock(BOOLEAN bCtrlHook);
		void			unlock(BOOLEAN bCtrlHook);

		void*			hookMalloc(size_t size, UINT uiClassID);
		void			hookFree(void* p);

		EBlockStatus	checkBlock(PBlockHeader pHeader);
		ELinkStatus		checkAllBlock(UINT &uiErrorBlockCount);
		void			linkBlock(PBlockHeader pHeader);
		void			unlinkBlock(PBlockHeader pHeader);

		BOOLEAN			outputStateToLogger(CLogger* pLogger, UINT uiGPUMemorySize);

		void			initSizeRange();
		INT				calcRangeIndex(size_t size);
		void			logAllocSize(size_t size);
		void			logFreedSize(size_t size);

		void			buildClassStat(MapThreadStat &mapThreadStat);

	public: // property

	private:// property
		CMemAllocator*	m_pMemAllocator;

		String			m_strReportFile;
		UINT			m_uiReportID;

		PBlockHeader	m_pBlockList;

		BOOLEAN			m_bCompactSizeRange;
		tagBlockStat	m_blockStatAll;
		tagBlockStat	m_blockStatSize[SIZE_INFO_MAX_COUNT];
		UINT			m_uiBadPtrAccess;

		CSyncObject		m_syncObject;
		INT				m_iHookLock;

		UINT			m_uiThreadCount;
		tagThreadSize	m_threadSize[SIZE_INFO_MAX_COUNT];

		Map<UINT, String, tagNumCompare<UINT> >		m_mapClassName;
        Map<UINT, String, tagNumCompare<UINT> >		m_mapThreadName;
	};
}

#endif