#include "BASE_Core.h"

#ifdef OS_QNX
	#include <wchar.h>
	#include <malloc.h>
#endif

#if defined(OS_LINUX) || defined(OS_ANDROID)
	#include <wchar.h>
	#include <malloc.h>
#endif

#ifdef OS_WIN
#endif

namespace BASE
{
	#define SIZE_UNIT_NODE_HEARDER						sizeof(tagUnitNode)
	#define SIZE_POOL_BLOCK_HEARDER						sizeof(tagPoolBlock)

	#define PTR_UNIT_NODE_HEADER(ptr_unit_data)			PUnitNode((BYTE*)ptr_unit_data - SIZE_UNIT_NODE_HEARDER)
	#define PTR_UNIT_NODE_DATA(ptr_unit_hdr)			(void*)((BYTE*)ptr_unit_hdr + SIZE_UNIT_NODE_HEARDER)

	#define PTR_POOL_BLOCK_DATA(ptr_hdr, data_offset)	(void*)((BYTE*)ptr_hdr + data_offset)

	#define UNIT_NODE_MAGIC								0x837a46c2
	#define MAKE_UNIT_NODE_MAGIC(ptr_unit_hdr)			(UNIT_NODE_MAGIC ^ (UINT)ptr_unit_hdr)

	#define MEM_ALIGN_BYTE_DEFAULT						((SIZE)4U)
	#define MEM_ALIGN_MASK_DEFAULT						(MEM_ALIGN_BYTE_DEFAULT - (SIZE)1U)

	#define MEM_ALIGN_PAD_SIZE(req_size, align_mask)	((req_size + (SIZE)align_mask) & ~(SIZE)align_mask)

	#define PINT_SET_VALUE(ptr_int, ivalue)	\
		if (ptr_int != NULL) \
		{ \
			*ptr_int = ivalue; \
		}

	////////////////////////////////////////////////////////////////////////////////////////////

	CMemAllocator::CMemAllocator()
		: m_pAryMemPool(NULL)
		, m_uiMaxPoolCount(0)
		, m_uiCurPoolCount(0)
	{
		m_uiMemAlignMask = MEM_ALIGN_MASK_DEFAULT;
		m_uiBlockDataOffset = MEM_ALIGN_PAD_SIZE(SIZE_POOL_BLOCK_HEARDER + SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask) - SIZE_UNIT_NODE_HEARDER;
		m_uiSysChunkHdrSize = MEM_ALIGN_PAD_SIZE(SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask);

		wprintf(L"[Info][BASE] Memory allocator started.\n");
	}

	CMemAllocator::~CMemAllocator()
	{
		freeAllPool();

		wprintf(L"[Info][BASE] Memory allocator exited.\n");
	}

	void CMemAllocator::initialize(UINT uiMemAlignByte, UINT uiMaxPoolCount)
	{
		if (uiMemAlignByte > 0)
		{
			m_uiMemAlignMask = ((uiMemAlignByte + 1) & ~1) - 1;
			m_uiSysChunkHdrSize = MEM_ALIGN_PAD_SIZE(SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask);
			m_uiBlockDataOffset = MEM_ALIGN_PAD_SIZE(SIZE_POOL_BLOCK_HEARDER + SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask) - SIZE_UNIT_NODE_HEARDER;
		}

		if (uiMaxPoolCount == 0)
		{
			return;
		}

		m_uiMaxPoolCount = uiMaxPoolCount;

		UINT uiMemSize = sizeof(tagMemPool) * m_uiMaxPoolCount;
		m_pAryMemPool = (tagMemPool*)::malloc(uiMemSize);
		memset(m_pAryMemPool, 0x0, uiMemSize);
	}

	BOOLEAN CMemAllocator::createPool(UINT uiUnitSize, UINT uiInitCount, UINT uiMaxCount, UINT uiAppendCount)
	{
		if (m_uiCurPoolCount >= m_uiMaxPoolCount)
		{
			return FALSE;
		}

		UINT uiUnitChunkSize = MEM_ALIGN_PAD_SIZE(uiUnitSize + SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask);
		UINT uiUnitAvailSize = uiUnitChunkSize - SIZE_UNIT_NODE_HEARDER;

		BOOLEAN bCreateNew = TRUE;
		INT iPoolIndex = -1;
		PMemPool pMemPool = findFitPool(uiUnitAvailSize, &iPoolIndex);

		if (pMemPool == NULL)
		{
			iPoolIndex = m_uiCurPoolCount;
		}
		else // Need insert or merge
		{
			if (pMemPool->m_uiUnitAvailSize == uiUnitAvailSize) // Merge
			{
				bCreateNew = FALSE;
				if (pMemPool->m_uiMaxCount != 0)
				{
					pMemPool->m_uiMaxCount += uiMaxCount;
					if (pMemPool->m_uiAppendCount < uiAppendCount)
					{
						pMemPool->m_uiAppendCount = uiAppendCount;
					}
				}
				wprintf(L"[Warning][BASE] Pool config item was merged to an existing one, (unit_size = %u).\n", uiUnitSize);
			}
			else // Insert
			{
				// Move array
				INT iTempIndex = m_uiCurPoolCount - 1;
				while (iTempIndex >= iPoolIndex)
				{
					memcpy(m_pAryMemPool + iTempIndex + 1, m_pAryMemPool + iTempIndex, sizeof(tagMemPool));
					--iTempIndex;
				}
			}
		}

		if (bCreateNew)
		{
			m_pAryMemPool[iPoolIndex].m_uiUnitChunkSize	= uiUnitChunkSize;
			m_pAryMemPool[iPoolIndex].m_uiUnitAvailSize	= uiUnitAvailSize;
			m_pAryMemPool[iPoolIndex].m_uiInitCount		= uiInitCount;
			m_pAryMemPool[iPoolIndex].m_uiMaxCount		= uiMaxCount;
			m_pAryMemPool[iPoolIndex].m_uiAppendCount	= uiAppendCount;
			m_pAryMemPool[iPoolIndex].m_uiCurrentCount	= 0;
			m_pAryMemPool[iPoolIndex].m_pFirstBlock		= NULL;
			m_pAryMemPool[iPoolIndex].m_pFreedLink		= NULL;

			// If we want to initialize the first block for each pool,
			// Please open the following code.
			//addPoolBlock(&m_pAryMemPool[m_uiCurPoolCount]);

			m_uiCurPoolCount++;
		}

		return TRUE;
	}

	BOOLEAN CMemAllocator::getPoolState(UINT uiIndex, tagMemPoolState& poolState)
	{
		if (uiIndex >= m_uiCurPoolCount)
		{
			return FALSE;
		}

		CAutoSync autoSync(m_syncObject);

		PMemPool pMemPool = m_pAryMemPool + uiIndex;

		poolState.uiUnitAvailSize = pMemPool->m_uiUnitAvailSize;
		poolState.uiMaxCount = pMemPool->m_uiMaxCount;
		poolState.uiCurrentCount = pMemPool->m_uiCurrentCount;

		poolState.uiFreeCount = 0;
		if (pMemPool->m_uiCurrentCount > 0)
		{
			PUnitNode pUnitNode = pMemPool->m_pFreedLink;
			while (pUnitNode)
			{
				++poolState.uiFreeCount;
				pUnitNode = pUnitNode->pNextUnit;
			}
			poolState.uiFreeCount += (pMemPool->m_pFirstBlock->uiUnitCount - pMemPool->m_pFirstBlock->uiUsedCursor);
		}

		poolState.uiMemoryCost = 0;
		if (pMemPool->m_uiCurrentCount > 0)
		{
			PPoolBlock pPoolBlock = pMemPool->m_pFirstBlock;
			while (pPoolBlock)
			{
				poolState.uiMemoryCost += pPoolBlock->uiBlockSize;
				pPoolBlock = pPoolBlock->pNextBlock;
			}
		}

		return TRUE;
	}

	void* CMemAllocator::malloc(SIZE size)
	{
		void* pRet = NULL;

		PMemPool pMemPool = findFitPool(size);
		if (pMemPool != NULL)
		{
			pRet = allocUnit(pMemPool);

			if (pRet != NULL)
			{
				//wprintf(L"---allocated: 0x%x\n", pRet);
				return pRet;
			}
		}

		pRet = ::malloc(m_uiSysChunkHdrSize + size); // make sure memory alignment

		if (pRet != NULL)
		{
			PUnitNode pUnitNode = PUnitNode((BYTE*)pRet + m_uiSysChunkHdrSize - SIZE_UNIT_NODE_HEARDER);

			pUnitNode->pMemPool = NULL;
			pUnitNode->uiMagic = MAKE_UNIT_NODE_MAGIC(pUnitNode);

			return PTR_UNIT_NODE_DATA(pUnitNode);
		}

		return NULL;
	}

	void CMemAllocator::free(void* p)
	{
		tagUnitNode* pUnitNode = PTR_UNIT_NODE_HEADER(p);

		if (pUnitNode->uiMagic != MAKE_UNIT_NODE_MAGIC(pUnitNode))
		{
			wprintf(L"[Warning][BASE] Free(0x%x): Invalid Pointer, Maybe it had been freed or crashed.\n", p);
			return;
		}

		if (pUnitNode->pMemPool != NULL)
		{
			CAutoSync autoSync(m_syncObject);

			pUnitNode->pNextUnit = pUnitNode->pMemPool->m_pFreedLink;
			pUnitNode->pMemPool->m_pFreedLink = pUnitNode;
		}
		else
		{
			::free((BYTE*)p - m_uiSysChunkHdrSize);
		}
	}

	BOOLEAN  CMemAllocator::addPoolBlock(tagMemPool* pMemPool)
	{
		if ((pMemPool->m_uiMaxCount > 0) &&
			(pMemPool->m_uiCurrentCount >= pMemPool->m_uiMaxCount))
		{
			return FALSE;
		}

		UINT uiAddCount = 0;
		if (pMemPool->m_pFirstBlock == NULL && pMemPool->m_uiInitCount != 0) // Initialize
		{
			uiAddCount = pMemPool->m_uiInitCount;
		}
		else // Append
		{
			uiAddCount = pMemPool->m_uiAppendCount;
		}
		if ((pMemPool->m_uiMaxCount > 0) &&
			(uiAddCount + pMemPool->m_uiCurrentCount > pMemPool->m_uiMaxCount))
		{
			uiAddCount = pMemPool->m_uiMaxCount - pMemPool->m_uiCurrentCount;
		}
		if (uiAddCount == 0)
		{
			wprintf(L"[Error][BASE] CMemAllocator::addPoolBlock: Pool(index = %u) config has error, please check Memory.ini.\n",
					(m_pAryMemPool - pMemPool) / sizeof(tagMemPool) + 1);
			return FALSE;
		}

		UINT uiBlockSize =	MEM_ALIGN_PAD_SIZE(SIZE_POOL_BLOCK_HEARDER + SIZE_UNIT_NODE_HEARDER, m_uiMemAlignMask)
							+ pMemPool->m_uiUnitChunkSize * uiAddCount - SIZE_UNIT_NODE_HEARDER;
		PPoolBlock pPoolBlock = (PPoolBlock)::malloc(uiBlockSize);
		if (pPoolBlock == NULL)
		{
			wprintf(L"[Warning][BASE] Allocate memory pool block failed (size = %u).\n", uiBlockSize);
			return FALSE;
		}

		pPoolBlock->uiBlockSize = uiBlockSize;
		pPoolBlock->uiUnitCount = uiAddCount;
		pPoolBlock->uiUsedCursor = 0;
		pPoolBlock->pNextBlock = pMemPool->m_pFirstBlock;

		pMemPool->m_pFirstBlock = pPoolBlock;
		pMemPool->m_uiCurrentCount += uiAddCount;

		return TRUE;
	}

	CMemAllocator::tagMemPool* CMemAllocator::findFitPool(SIZE size, INT* piIndex)
	{
		if (m_uiCurPoolCount == 0)
		{
			PINT_SET_VALUE(piIndex, -1);
			return NULL;
		}

		INT iHigh = m_uiCurPoolCount - 1;

		if (size >= m_pAryMemPool[iHigh].m_uiUnitAvailSize)
		{
			if (size == m_pAryMemPool[iHigh].m_uiUnitAvailSize)
			{
				PINT_SET_VALUE(piIndex, iHigh);
				return &m_pAryMemPool[iHigh];
			}

			PINT_SET_VALUE(piIndex, -1);
			return NULL;
		}

		INT iLow = 0;
		INT iMiddle = m_uiCurPoolCount / 2;

		while(iHigh >= iLow)
		{
			iMiddle = (iHigh + iLow) / 2;
			SIZE uiAvailSize = m_pAryMemPool[iMiddle].m_uiUnitAvailSize;

			if (size > uiAvailSize)
			{
				iLow = iMiddle + 1;
			}
			else if (size < uiAvailSize)
			{
				iHigh = iMiddle - 1;
			}
			else // ==
			{
				iLow = iMiddle;
				break;
			}
		}

		PINT_SET_VALUE(piIndex, iLow);
		return &m_pAryMemPool[iLow];
	}

	void CMemAllocator::freeAllPool()
	{
		UINT uiIndex = 0;
		while (uiIndex < m_uiCurPoolCount)
		{
			tagMemPool* pMemPool = m_pAryMemPool + uiIndex;

			PPoolBlock pPoolBlock = pMemPool->m_pFirstBlock;
			while (pPoolBlock)
			{
				PPoolBlock pTempBlock = pPoolBlock;
				pPoolBlock = pPoolBlock->pNextBlock;
				::free(pTempBlock);
			}
			pMemPool->m_pFirstBlock = NULL;

			++uiIndex;
		}

		m_uiCurPoolCount = 0;

		::free(m_pAryMemPool);
		m_pAryMemPool = NULL;
		m_uiMaxPoolCount = 0;
	}

	void* CMemAllocator::allocUnit(tagMemPool* pMemPool)
	{
		CAutoSync autoSync(m_syncObject);

		PUnitNode pUnitNode = pMemPool->m_pFreedLink;

		if (pUnitNode != NULL)
		{
			pMemPool->m_pFreedLink = pUnitNode->pNextUnit;
			pUnitNode->uiMagic = MAKE_UNIT_NODE_MAGIC(pUnitNode);

			return PTR_UNIT_NODE_DATA(pUnitNode);
		}

		tagPoolBlock* pPoolBlock = pMemPool->m_pFirstBlock;

		if (pPoolBlock == NULL || pPoolBlock->uiUsedCursor >= pPoolBlock->uiUnitCount)
		{
			if (addPoolBlock(pMemPool))
			{
				pPoolBlock = pMemPool->m_pFirstBlock;
			}
			else
			{
				return NULL;
			}
		}

		pUnitNode = PUnitNode((BYTE*)PTR_POOL_BLOCK_DATA(pPoolBlock, m_uiBlockDataOffset) + pMemPool->m_uiUnitChunkSize * pPoolBlock->uiUsedCursor);
		pPoolBlock->uiUsedCursor++;

		pUnitNode->uiMagic = MAKE_UNIT_NODE_MAGIC(pUnitNode);
		pUnitNode->pMemPool = pMemPool;

		return PTR_UNIT_NODE_DATA(pUnitNode);
	}
}
