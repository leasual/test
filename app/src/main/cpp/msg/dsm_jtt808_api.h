//
// Created by public on 19-3-27.
//

#ifndef DSM_JTT808_DSM_JTT808_API_H
#define DSM_JTT808_DSM_JTT808_API_H


#include "base/config_file.h"
#include "hp_socket_helper.h"
#include "client_conn.h"
#include "client_conn_manager.h"

extern "C" {

CClientConn *g_objClientConn = nullptr;
unsigned int *g_nClientFd = nullptr;





void SetGpsInfo(uint64_t latitude, uint64_t longitude, uint32_t height) {
    CClientConnManager::GetInstance()->SetLocation(*g_nClientFd, latitude, longitude, height);
}

void UnInialise() {
    HPSocketHelper::DestroyHPSocketObjects();
    *g_nClientFd = -1;
    delete g_objClientConn;
}

#endif //DSM_JTT808_DSM_JTT808_API_H
}