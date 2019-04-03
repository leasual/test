//
// Created by public on 19-3-19.
//

#include "util.h"

CUtil::CUtil()
{

}

CUtil::~CUtil()
{

}

void CUtil::writePid()
{
    uint32_t curPid;
    curPid = (uint32_t) getpid();
    FILE* f = fopen("server.pid", "w");
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d", curPid);
    fwrite(szPid, strlen(szPid), 1, f);
    fclose(f);
}

uint64_t CUtil::get_tick_count()
{
    struct timeval tval;
    uint64_t ret_tick;

    gettimeofday(&tval, NULL);

    ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return ret_tick;
}


bool CUtil::ReadFile(const char* lpszFileName, CFile& file, CFileMapping& fmap, DWORD dwMaxFileSize)
{
    if (NULL == lpszFileName)
        return false;

    if(file.Open(lpszFileName, O_RDONLY))
    {
        SIZE_T dwSize = 0;
        if(file.GetSize(dwSize))
        {
            if(dwSize > 0 && dwSize <= dwMaxFileSize)
            {
                if(fmap.Map(file, dwSize))
                    return true;
            }
            else if(dwSize == 0)
                UT_ERROR("File size is zero.");
            else
                UT_ERROR("File is too large.");
        }
    }else{
        UT_FATAL("Open file[%s] failed!",lpszFileName);
    }
    return false;
}