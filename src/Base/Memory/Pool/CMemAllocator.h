/**
* @attention Copyright
* Copyright - All Rights Reserved
* HARMAN Shanghai - HMI Team of Software
* Email: Feng.Wang@harman.com
*/

#ifndef __BASE_CMEMALLOCATOR_H_INCLUDED__
#define __BASE_CMEMALLOCATOR_H_INCLUDED__

namespace BASE
{
	class CMemAllocator
	{
	public:// const define

	private:// const define

	public:// embed class or struct or enum

		struct tagMemPoolState
		{
			UINT	uiUnitAvailSize;
			UINT	uiMaxCount;
			UINT	uiCurrentCount;
			UINT	uiFreeCount;
			UINT	uiMemoryCost;
			//UINT	uiWasteCost;
		};

	private:// embed class or struct or enum
		struct tagMemPool;

		struct tagUnitNode
		{
			tagMemPool*			pMemPool;
			union
			{
				tagUnitNode*	pNextUnit;
				UINT			uiMagic;
			};
		};
		typedef tagUnitNode* PUnitNode;

		struct tagPoolBlock
		{
			UINT			uiBlockSize;

			UINT			uiUnitCount;
			UINT			uiUsedCursor;

			tagPoolBlock*	pNextBlock;
		};
		typedef tagPoolBlock* PPoolBlock;

		struct tagMemPool
		{
			UINT			m_uiUnitChunkSize;
			UINT			m_uiUnitAvailSize;

			UINT			m_uiInitCount;
			UINT			m_uiMaxCount; // "MaxCount == 0" means no limited.
			UINT			m_uiAppendCount;
			UINT			m_uiCurrentCount;

			tagPoolBlock*	m_pFirstBlock;
			tagUnitNode*	m_pFreedLink;
		};
		typedef tagMemPool* PMemPool;

	public:// method
		// inner method
		CMemAllocator();
		virtual ~CMemAllocator();

		void			initialize(UINT uiMemAlignByte, UINT uiMaxPoolCount);

		BOOLEAN			createPool(UINT uiUnitSize, UINT uiInitCount, UINT uiMaxCount, UINT uiAppendCount);
		UINT			getPoolCount() { return m_uiCurPoolCount; };
		BOOLEAN			getPoolState(UINT uiIndex, tagMemPoolState& poolState);

		void*			malloc(size_t size);
		void			free(void* p);

	protected:
		// Inner method

	private: // method
		BOOLEAN			addPoolBlock(tagMemPool* pMemPool);
		tagMemPool*		findFitPool(size_t size, INT* piIndex = NULL);
		void			freeAllPool();

		void*			allocUnit(tagMemPool* pMemPool);

	public: // property

	private:// property
		tagMemPool*		m_pAryMemPool;
		UINT			m_uiMaxPoolCount;
		UINT			m_uiCurPoolCount;

		UINT			m_uiMemAlignMask;

		UINT			m_uiBlockDataOffset;
		UINT			m_uiSysChunkHdrSize;

		CSyncObject		m_syncObject;
	};
}

#endif