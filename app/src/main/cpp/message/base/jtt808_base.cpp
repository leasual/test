//
// Created by public on 19-3-20.
//

#include "jtt808_base.h"
#include <hpsocket/common/GlobalDef.h>

JTT808PktBase::JTT808PktBase():_dataptr(0)
{
    memset(_buffer,0,sizeof(_buffer));
}

JTT808PktBase::~JTT808PktBase()
{

}


/**
* 接收到数据
*/
bool JTT808PktBase::ReceiveData(BYTE* buffer , int bufferlen)
{
    _lastrecvtime = time(NULL) ;
    if( bufferlen + _dataptr > BUFFER_SIZE )
        return false ;
    memcpy(_buffer + _dataptr, buffer, bufferlen ) ;
    _dataptr += bufferlen ;

    return true ;
}


void JTT808PktBase::ProcessData()
{
    return;
}

/**
* 无条件丢弃iScan字节
*/
void JTT808PktBase::Scan(int iScan)
{
    if(0 == _dataptr)
        return ;
    memcpy(_buffer, _buffer +iScan, _dataptr -iScan);
    _dataptr -= iScan ;
}


void JTT808PktBase::ScanInvalidData()
{
    int i = 0 ;
    while( _buffer[i] != 0x7e && i < _dataptr ){i++;} ;
    if( i == _dataptr )
    {
        _dataptr = 0 ;
        return ;
    }
    if(i != 0)
    {
        memcpy( _buffer , _buffer + i ,  _dataptr -i )   ;
        _dataptr -= i ;
    }
}

bool JTT808PktBase::GetOneRawPacket()
{
    m_nRecvRawPtrOffset = 0 ;
    int iPos = 0 ;
    ScanInvalidData() ;

    /*异常短包*/
//    if( _dataptr <  sizeof(JT808Protocol) )
//        return false;

    for(int i = 1 ; i < _dataptr; i++)
    {
        /*获取到一个符合头尾标志位的包*/
        if(_buffer[i] == MSG_FLAG)
        {
            iPos = i ;
            memcpy(m_szRecvRawOnePkt , _buffer , iPos+1) ;
            m_nRecvRawPtrOffset = iPos + 1 ;
            Scan(iPos+1);
            return true ;
        }
    }
    /*异常长包*/
    if( _dataptr > MAX_PACKET_LEN )
        Scan(_dataptr) ;
    return false ;
}

//
// 生成流水号
//
int  JTT808PktBase::GenerateSeqNo()
{
//    srand(time(NULL));
//    int nSeqNo = rand()%(0xFFFF - 0x0001) + 0x0001;
//    char buff[48] = {0};
//    sprintf(buff,"%x",nSeqNo);
//    UT_TRACE("The seqNo is[%d] [%x]",nSeqNo,nSeqNo);

    static uint64_t  nIndex = 0;

    WORD nSeqNo = nIndex++ % MAXWORD;
    char buff[48] = {0};
    sprintf(buff,"%x",nSeqNo);
    //UT_TRACE("The seqNo is[%d] [%x]",nSeqNo,nSeqNo);
    return nSeqNo;
}

//
// 字符串转换成BCD码
//
void JTT808PktBase::Str2BCD(char*t, char* str)
{
    int   i, len;

    len = strlen(str);
    for( i = 0; i < len; i += 2, t++ )
    {
        *t = ((str[len - i - 2] - 0x30) << 4) | (str[len - i - 1] - 0x30);
    }
    *t = 0;
}

//
// 取得一个转义后的报文
//
bool JTT808PktBase::GetOnePacket(WORD* cmd)
{
    if( !GetOneRawPacket() )
        return false ;
    /*恢复转义*/
    RecvBufferTransfer();
    UT_TRACE("===Recived packet===");
    UT_DUMP(m_szRecvOnePkt,m_nRecvPktOffset);
    m_nRecvRawPtrOffset = 0  ;

    /*校验*/
    //UT_TRACE("Check code[%d]",m_szRecvOnePkt[m_nRecvPktOffset-1]);
    if (MakeCheckProtocol((BYTE*)m_szRecvOnePkt,m_nRecvPktOffset-1) == (BYTE)m_szRecvOnePkt[m_nRecvPktOffset-1]) {
        *cmd = (m_szRecvOnePkt[0]<<8) + m_szRecvOnePkt[1];
        UT_TRACE("Command[%x]  Check OK", *cmd);
        return true;
    }

    m_nRecvPktOffset = 0 ;
    return false ;
}


//
// 收到的包去掉转义,同时去掉头尾标识
//
void JTT808PktBase::RecvBufferTransfer()
{
    m_nRecvPktOffset = 0 ;
    int iTempPos = 0 ;
    int i = 0 ;
    for(i = 1 ,iTempPos = 0 ; i< m_nRecvRawPtrOffset-1 ; iTempPos++ )
    {
        if( m_szRecvRawOnePkt[i] == 0x7d && m_szRecvRawOnePkt[i+1] == 0x02) {
            m_szRecvOnePkt[iTempPos] = 0x7e;
            i+=2;
        }
        else if( m_szRecvRawOnePkt[i] == 0x7d && m_szRecvRawOnePkt[i+1] == 0x01 ){
            m_szRecvOnePkt[iTempPos] = 0x7d;
            i+=2;
        }
        else{
            m_szRecvOnePkt[iTempPos] = m_szRecvRawOnePkt[i];
            i++;
        }
    }
    //m_szRecvOnePkt[iTempPos] = m_szRecvRawOnePkt[i] ;  //0x7e
    m_nRecvPktOffset = iTempPos ;
}


//
// 加入转义
//
void JTT808PktBase::ReqBufferTransfer() //上行包加入转义
{
    m_reqRawPtr = 0 ;
    int iTempPos = 0 ;
    int i = 0 ;
    m_szReqRawOneBuffer[0] = MSG_FLAG ;
    for(  i = 0 ,iTempPos = 1 ; i< m_reqPtrOffset; i ++ )
    {
        if( m_szReqOneBuffer[i] == MSG_FLAG ) {
            m_szReqRawOneBuffer[iTempPos] = 0x7d ;
            m_szReqRawOneBuffer[iTempPos+1] = 0x02;
            iTempPos+=2 ;
        }
        else if( m_szReqOneBuffer[i] == 0x7d ) {
            m_szReqRawOneBuffer[iTempPos] = 0x7d ;
            m_szReqRawOneBuffer[iTempPos+1] = 0x01 ;
            iTempPos+=2 ;
        }
        else{
            m_szReqRawOneBuffer[iTempPos] = m_szReqOneBuffer[i] ;
            iTempPos++ ;
        }
    }

    m_szReqRawOneBuffer[iTempPos] = MSG_FLAG ;  //0x7e
    m_reqRawPtr = iTempPos+1 ;
}


BYTE JTT808PktBase::make_check(BYTE *psrc, unsigned int ilen)
{
    BYTE btResult = psrc[0] ;
    for(int i=1; i<ilen ; i++ )
    {
        btResult ^= psrc[i] ;
    }
    return btResult;
}


BYTE JTT808PktBase::MakeCheckProtocol(BYTE *pData,int nLen)
{
    //核对校验
    BYTE check = pData[0];
    /* calulate the check */
    for(int i=1;i<nLen;i++)
    {
        check ^= pData[i];
    }
    //UT_TRACE("check code[%x]",check);
    return check;
}


BYTE JTT808PktBase::BCDtoNumber(const BYTE& bcd)
{
    return (0xff & (bcd>>4))*10 +(0xf & bcd);
}
