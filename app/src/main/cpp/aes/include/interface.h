//*****************************************************************************
//	File name  : 				interface.h
//
//	Description:  This file provides all functions prototypes of the serial bus operation.
//	
//  written by :  jason
//	History    :
//                 2015/04/13      jason       v0.1    Initial version
//                 2015/05/31      jason       v1.0    added iSleep function
//******************************************************************************
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "type.h"

BOOL iDeviceInit(uint8_t device_addr, uint8_t speed);
BOOL iDeviceDeInit();
BOOL iWriteByte(uint8_t addr, uint8_t data);
BOOL iReadByte(uint8_t addr, uint8_t *data);
BOOL iSleep(uint8_t waittime);

#define TRUE (0)
#define FALSE (-1)
#endif
