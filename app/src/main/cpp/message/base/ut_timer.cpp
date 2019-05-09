//
// Created by public on 19-4-19.
//

#include "ut_timer.h"
#include "util.h"
#include "log4z.h"


UTTimer::UTTimer():m_bIsRunning(false)
{

}

UTTimer::~UTTimer()
{

}

void UTTimer::AddTimer(std::function<void(uint64_t)>* callback,DWORD interval)
{
    if (_FindTimer(callback)) {
        UT_WARN("Callback already in the list.");
        return;
    }
    TimerItem* pItem = new TimerItem;
    pItem->callBack = callback;
    pItem->interval = interval;
    pItem->next_tick = CUtil::GetInstance()->get_tick_count() + interval;
    m_timer_list.push_back(pItem);
}

bool UTTimer::_FindTimer(std::function<void(uint64_t)>* callback)
{
    std::list<TimerItem*>::iterator it;
    bool bFoundCallback = false;
    for (it = m_timer_list.begin(); it != m_timer_list.end(); it++)
    {
        TimerItem* pItem = *it;
        if (pItem->callBack == callback)
        {
            bFoundCallback = true;
            break;
        }
    }
    return bFoundCallback;
}

void UTTimer::RemoveTimer(std::function<void(uint64_t)>* callback)
{
    std::list<TimerItem*>::iterator it;
    bool bFoundCallback = false;
    for (it = m_timer_list.begin(); it != m_timer_list.end(); it++)
    {
        TimerItem* pItem = *it;
        if (pItem->callBack == callback)
        {
            m_timer_list.erase(it);
            delete pItem;
            bFoundCallback = true;
            break;
        }
    }

    if (bFoundCallback)
        UT_TRACE("Remove callback success!");
    else
        UT_TRACE("Remove callback failed!");
}

void UTTimer::StartDispatch(uint32_t wait_timeout)
{
    if(m_bIsRunning)
        return;
    m_bIsRunning = true;
    while (m_bIsRunning) {
        CheckTimer();
        usleep(wait_timeout * 1000); // 让出millSecond毫秒的时间片
    }
}

void UTTimer::CheckTimer()
{
    uint64_t curr_tick = CUtil::GetInstance()->get_tick_count();
    std::list<TimerItem*>::iterator it;
    for (it = m_timer_list.begin(); it != m_timer_list.end();)
    {
        TimerItem* pItem = *it;
        it++;		// iterator maybe deleted in the callback, so we should increment it before callback
        if (curr_tick >= pItem->next_tick)
        {
            // 已经超时了
            pItem->next_tick += pItem->interval;
            (*pItem->callBack)(CUtil::GetInstance()->get_tick_count());
        }
    }
}

void UTTimer::StopDispatch()
{
    m_bIsRunning = false;
}