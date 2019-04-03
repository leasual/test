//
// Created by public on 19-3-20.
//

#ifndef DSM_JTT808_PKT_UTIL_H
#define DSM_JTT808_PKT_UTIL_H

#include "public_def.h"

class JTT808PktBase
{
public:
    enum{ BUFFER_SIZE = 40960 }; /**< 缓冲区大小  */
    enum { MAX_PACKET_LEN  = 64*1024 };
    JTT808PktBase();
    virtual  ~JTT808PktBase();

    virtual bool ReceiveData(BYTE *buffer, int bufferlen);
    virtual void ProcessData() ;

protected:
    void ScanInvalidData();
    void Scan(int iScan = 1);
    bool GetOneRawPacket();
    bool GetOnePacket(WORD* cmd) ;
    void RecvBufferTransfer();    //下行包去掉转义
    void ReqBufferTransfer();  //上行包加入转义
    BYTE BCDtoNumber(const BYTE& bcd); /*Util  : BCD->NUM */
    int  GenerateSeqNo();
    void Str2BCD(char*t, char* str);
    BYTE MakeCheckProtocol(BYTE *pData,int nLen);

protected:
    /*预定义缓冲区，提高处理效率*/
    char m_szRecvOnePkt[MAX_PACKET_LEN];      // 一个转义后的包缓冲
    int  m_nRecvPktOffset ;                         //转义后指针
    char m_szRecvRawOnePkt[MAX_PACKET_LEN]  ; //一个未转义原始包缓冲
    int  m_nRecvRawPtrOffset ;                         //原始指针

    char m_szReqOneBuffer[MAX_PACKET_LEN]  ;    //一个未转义的请求包缓冲
    int  m_reqPtrOffset ;                            //未转义请求包指针

    char m_szReqRawOneBuffer[MAX_PACKET_LEN]  ; //一个转义后的请求包缓冲
    int  m_reqRawPtr;                         //转义后的请求包指针
    BYTE   _buffer[BUFFER_SIZE] ; /**< 缓冲区 */
    int    _dataptr ;      /**< 数据指针 */
    time_t _lastrecvtime ;     /**< 最后接收时间 */
    time_t _lastsendtime ; /**< 最后发送时间 */

private:
    BYTE make_check(BYTE *psrc, unsigned int ilen);

};


#endif //DSM_JTT808_PKT_UTIL_H
