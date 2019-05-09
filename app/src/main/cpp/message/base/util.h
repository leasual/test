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

static const unsigned short crc16tab[256]= {
        0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
        0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
        0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
        0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
        0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
        0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
        0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
        0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
        0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
        0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
        0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
        0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
        0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
        0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
        0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
        0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
        0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
        0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
        0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
        0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
        0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
        0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
        0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
        0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
        0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
        0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
        0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
        0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
        0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
        0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
        0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
        0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};


class CUtil : public Singleton<CUtil>
{
public:
    CUtil();
    ~CUtil();
    void writePid();

    uint64_t get_tick_count();

    bool ReadFile(const char* lpszFileName, CFile& file, CFileMapping& fmap, DWORD dwMaxFileSize = MAX_SMALL_FILE_SIZE);

    const std::string&& GetCurrentTm();  // 取得当前的时间

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

    DWORD BCD2ASC (const BYTE *bcd,BYTE *asc, DWORD len)
    {
        BYTE bcd2ascii[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        BYTE c = 0;
        BYTE i;

        for(i = 0; i < len; i++) {
            //first BCD
            c = *bcd >> 4;
            *asc++ = bcd2ascii[c];

            //second
            c = *bcd & 0x0f;
            *asc++ = bcd2ascii[c];
            bcd++;
        }
        return 0;
    }


    DWORD ASC2BCD(BYTE *bcd, const BYTE *asc, DWORD len)
    {
        BYTE ascii2bcd1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        BYTE ascii2bcd2[6]  = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
        BYTE c = 0;
        BYTE index = 0;
        BYTE i = 0;

        len >>= 1;

        for(; i < len; i++) {
            //first BCD
            if(*asc >= 'A' && *asc <= 'F') {
                index = *asc - 'A';
                c  = ascii2bcd2[index] << 4;
            } else if(*asc >= '0' && *asc <= '9') {
                index = *asc - '0';
                c  = ascii2bcd1[index] << 4;
            }
            asc++;

            //second BCD
            if(*asc >= 'A' && *asc <= 'F') {
                index = *asc - 'A';
                c  |= ascii2bcd2[index];
            } else if(*asc >= '0' && *asc <= '9') {
                index = *asc - '0';
                c  |= ascii2bcd1[index];
            }
            asc++;

            *bcd++ = c;
        }

        return 0;
    }

    unsigned short crc16_ccitt(const char *buf, int len);
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
