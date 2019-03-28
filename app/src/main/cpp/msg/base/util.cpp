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
