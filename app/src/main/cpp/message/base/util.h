//
// Created by public on 19-3-19.
//

#ifndef DSM_JTT808_UTIL_H
#define DSM_JTT808_UTIL_H

#include <sys/signal.h>
#include <unistd.h>
#include <time.h>
#include <atomic>
#include <utility>
#include <fcntl.h>
#include <malloc.h>
#include <alloca.h>
#include <assert.h>

#include <sys/time.h>
#include "singleton.h"
#include "public_def.h"
#include "file_helper.h"


using namespace std;

template<typename T, typename ... A>
inline T* ConstructObject(T* p, A&& ... args)
{
    return new (p) T(std::forward<A>(args) ...);
}


template<typename T>
inline void DestructObject(T* p)
{
    p->T::~T();
}

template<typename T>
inline T InterlockedCompareExchange(volatile T* _Tgt, T _Value, T _Exp, BOOL _bWeek = FALSE, std::memory_order m1 = std::memory_order_seq_cst, std::memory_order m2 = std::memory_order_seq_cst)
{
    __atomic_compare_exchange_n(_Tgt, &_Exp, _Value, _bWeek, m1, m2);
    return _Exp;
}

template<typename T, typename V, typename E, typename = enable_if_t<is_same<decay_t<T>, decay_t<V>>::value && is_same<decay_t<V>, decay_t<E>>::value>>
inline V* InterlockedCompareExchangePointer(volatile T** _Tgt, V* _Value, E* _Exp, BOOL _bWeek = FALSE, memory_order m1 = memory_order_seq_cst, memory_order m2 = memory_order_seq_cst)
{
    return (V*)(ULONG_PTR)InterlockedCompareExchange((volatile ULONG_PTR*)(volatile PVOID*)_Tgt, (ULONG_PTR)(PVOID)_Value, (ULONG_PTR)(PVOID)_Exp, _bWeek, m1, m2);
}

#define InterlockedExchangeAdd(p, n)	__atomic_add_fetch((p), (n), memory_order_seq_cst)
#define InterlockedExchangeSub(p, n)	__atomic_sub_fetch((p), (n), memory_order_seq_cst)
#define InterlockedIncrement(p)			InterlockedExchangeAdd((p), 1)
#define InterlockedDecrement(p)			InterlockedExchangeSub((p), 1)

class CUtil : public Singleton<CUtil>
{
public:
    CUtil();
    ~CUtil();
    void writePid();

    uint64_t get_tick_count();

    bool ReadFile(const char* lpszFileName, CFile& file, CFileMapping& fmap, DWORD dwMaxFileSize = MAX_SMALL_FILE_SIZE);

    std::string  Byte2Hex(BYTE bArray[], int bArray_len)
    {
        std::string strHex;
        int nIndex = 0;
        for(int i=0; i<bArray_len; i++)
        {
            int high = bArray[0]/16, low = bArray[0]%16;
            strHex[nIndex] = (high<10) ? ('0' + high) : ('A' + high - 10);
            strHex[ nIndex + 1] = (low<10) ? ('0' + low) : ('A' + low - 10);
            nIndex += 2;
        }
        return strHex;
    }

};


//自旋锁
class Spinlock
{
    std::atomic_flag flag;

public:
    Spinlock():flag(0){}

    void lock() {
        while(flag.test_and_set(std::memory_order_acquire));
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }
};



template <class T> class CCASQueueX
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

public:

    UINT Size()		{return m_iSize;}
    BOOL IsEmpty()	{return m_iSize == 0;}

    //void Lock()		{while(!TryLock()) _mm_pause();}
    void Lock()		{while(!TryLock()) usleep(1);}
    void Unlock()	{m_iLock = 0;}
    BOOL TryLock()	{return (::InterlockedCompareExchange(&m_iLock, 1u, 0u) == 0);}



public:

    CCASQueueX() : m_iLock(0), m_iSize(0)
    {
        m_pHead = m_pTail = new Node(nullptr);
    }

    ~CCASQueueX()
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
    CCASQueueX(const CCASQueueX&) = delete;
    CCASQueueX& operator = (const CCASQueueX&) = delete;

private:
    VUINT	m_iLock;
    VUINT	m_iSize;
    VNPTR	m_pHead;
    VNPTR	m_pTail;
    //Spinlock m_spinLock;  // 自旋锁
};


template<typename T, typename C, typename = enable_if_t<is_integral<T>::value  && (is_same<C, char>::value || is_same<C, wchar_t>::value)>>
C* _n_2_c(T value, C* lpszDest, int radix)
{
    static const C* dig = "0123456789abcdefghijklmnopqrstuvwxyz";

    bool neg = false;

    if(is_signed<T>::value && value < 0)
    {
        value = -value;
        neg	  = true;
    }

    int n = 0;

    do
    {
        lpszDest[n++] = dig[value % radix];
        value /= radix;
    } while(value);

    if(neg) lpszDest[n++] = '-';
    lpszDest[n]			  = 0;

    C c, *p, *q;
    for(p = lpszDest, q = p + n - 1; p < q; ++p, --q)
        c = *p, *p = *q, *q = c;

    return lpszDest;
}

#define itoa(v, p, r)		_n_2_c<INT, char>((v), (p), (r))



#endif //DSM_JTT808_UTIL_H
