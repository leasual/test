//
// Created by public on 19-4-18.
//

#ifndef DSM_JTT808_UT_QUEUE_H
#define DSM_JTT808_UT_QUEUE_H

#include "util.h"

template <typename T>
class UTQueue
{
private:
    struct Node;
    typedef Node*			NPTR;
    typedef volatile Node*	VNPTR;
    typedef volatile UINT	VUINT;

    struct Node
    {
        T*		pValue;
        VNPTR	pNext;

        Node(T* val, NPTR next = nullptr)
                : pValue(val), pNext(next)
        {

        }
    };

public:
    void PushBack(T* pVal)
    {
        ASSERT(pVal != nullptr);
        VNPTR pTail	= nullptr;
        NPTR pNode	= new Node(pVal);

        while(true)
        {
            pTail = m_pTail;

            if(::InterlockedCompareExchangePointer(&m_pTail, pNode, pTail) == pTail)
            {
                pTail->pNext = pNode;
                break;
            }
        }
        ::InterlockedIncrement(&m_iSize);
    }


    void UnsafePushBack(T* pVal)
    {
        ASSERT(pVal != nullptr);
        NPTR pNode		= new Node(pVal);
        m_pTail->pNext	= pNode;
        m_pTail			= pNode;

        ::InterlockedIncrement(&m_iSize);
    }

    BOOL FetchFront(T** ppVal)
    {
        ASSERT(ppVal != nullptr);
        if(IsEmpty())
            return FALSE;

        BOOL isOK	= FALSE;
        NPTR pHead	= nullptr;
        NPTR pNext	= nullptr;
        while(true)
        {
            Lock();
            pHead = (NPTR)m_pHead;
            pNext = (NPTR)pHead->pNext;

            if(pNext == nullptr)
            {
                Unlock();
                break;
            }
            *ppVal	= pNext->pValue;
            Unlock();

            isOK	= TRUE;
            break;
        }

        return isOK;
    }

    BOOL PopFront(T** ppVal)
    {
        ASSERT(ppVal != nullptr);
        if(IsEmpty())
            return FALSE;

        BOOL isOK	= FALSE;
        NPTR pHead	= nullptr;
        NPTR pNext	= nullptr;

        while(true)
        {
            Lock();
            pHead = (NPTR)m_pHead;
            pNext = (NPTR)pHead->pNext;

            if(pNext == nullptr)
            {
                Unlock();
                break;
            }

            *ppVal	= pNext->pValue;
            m_pHead	= pNext;

            Unlock();

            isOK	= TRUE;
            ::InterlockedDecrement(&m_iSize);
            delete pHead;
            break;
        }

        return isOK;
    }

    BOOL UnsafePopFront(T** ppVal)
    {
        if(!UnsafePeekFront(ppVal))
            return FALSE;

        UnsafePopFrontNotCheck();

        return TRUE;
    }

    BOOL UnsafePeekFront(T** ppVal)
    {
        ASSERT(ppVal != nullptr);
        NPTR pNext = (NPTR)m_pHead->pNext;

        if(pNext == nullptr)
            return FALSE;

        *ppVal = pNext->pValue;

        return TRUE;
    }

    void UnsafePopFrontNotCheck()
    {
        NPTR pHead	= (NPTR)m_pHead;
        NPTR pNext	= (NPTR)pHead->pNext;
        m_pHead		= pNext;

        ::InterlockedDecrement(&m_iSize);

        delete pHead;
    }

    void UnsafeClear()
    {
        ASSERT(m_pHead != nullptr);
        while(m_pHead->pNext != nullptr)
            UnsafePopFrontNotCheck();
    }

    UINT Size()		{return m_iSize;}
    BOOL IsEmpty()	{return m_iSize == 0;}

    //void Lock()		{while(!TryLock()) _mm_pause();}
    void Lock()		{while(!TryLock()) usleep(1);}
    void Unlock()	{m_iLock = 0;}
    BOOL TryLock()	{return (::InterlockedCompareExchange(&m_iLock, 1u, 0u) == 0);}

public:
    UTQueue() : m_iLock(0), m_iSize(0)
    {
        m_pHead = m_pTail = new Node(nullptr);
    }

    ~UTQueue()
    {
        ASSERT(m_iLock == 0);
        ASSERT(m_iSize == 0);
        ASSERT(m_pTail == m_pHead);
        ASSERT(m_pHead != nullptr);
        ASSERT(m_pHead->pNext == nullptr);

        UnsafeClear();

        delete m_pHead;
    }

private:
    UTQueue(const UTQueue&) = delete;
    UTQueue& operator = (const UTQueue&) = delete;

private:
    VUINT	m_iLock;
    VUINT	m_iSize;
    VNPTR	m_pHead;
    VNPTR	m_pTail;
    //Spinlock m_spinLock;  // 自旋锁
};

#endif //DSM_JTT808_UT_QUEUE_H
