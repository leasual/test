//
// Created by public on 19-4-19.
//

#ifndef DSM_JTT808_UT_TIMER_H
#define DSM_JTT808_UT_TIMER_H

#include "type_def.h"
#include "singleton.h"
#include <list>

class UTTimer :
        public Singleton<UTTimer>
{
public:
    UTTimer();
    ~UTTimer();

    void AddTimer(std::function<void(uint64_t)>* callback,DWORD interval);
    void RemoveTimer(std::function<void(uint64_t)>* callback);

    void StartDispatch(uint32_t wait_timeout = 200);
    void StopDispatch();
    void CheckTimer();
    bool IsRunning() {return m_bIsRunning;}

private:
    typedef struct {
        std::function<void(uint64_t)>* callBack;
        //void*           user_data;
        uint64_t        interval;
        uint64_t        next_tick;
    } TimerItem;
    std::list<TimerItem*>	m_timer_list;
    bool m_bIsRunning;

private:
    UTTimer(const UTTimer&) = delete;
    UTTimer&operator = (const UTTimer&) = delete;

    bool _FindTimer(std::function<void(uint64_t)>* callback);
};

#endif //DSM_JTT808_UT_TIMER_H
