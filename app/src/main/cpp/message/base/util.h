//
// Created by public on 19-3-19.
//

#ifndef DSM_JTT808_UTIL_H
#define DSM_JTT808_UTIL_H

#include <sys/signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "singleton.h"
#include "type_def.h"
#include "msg_def.h"
#include "file_helper.h"


class CUtil : public Singleton<CUtil>
{
public:
    CUtil();
    ~CUtil();
    void writePid();

    uint64_t get_tick_count();

    bool ReadFile(const char* lpszFileName, CFile& file, CFileMapping& fmap, DWORD dwMaxFileSize = MAX_SMALL_FILE_SIZE);
};

#endif //DSM_JTT808_UTIL_H
