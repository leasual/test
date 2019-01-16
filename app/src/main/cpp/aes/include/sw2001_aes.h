//*****************************************************************************
//	File name  : 				sw2001_aes.h
//
//	Description:  This file provides all functions prototypes of aes
//	
//  written by :  jason, iSmartware Technology Co.,LTD
//	History    :
//                 2015/04/13      jason       v0.1    Initial version
//******************************************************************************

#ifndef __SW2001_AES_H__
#define __SW2001_AES_H__
#include <memory.h>
#include "type.h"

//i2c device address
#define SW2001_DEVICE_ADDR 0x3C

//sw2001 cipher text start address
#define SW2001_REG_CIPHER_TEXT_ADDR 0x10

//sw2001 plaint text start address
#define SW2001_REG_PLAINT_TEXT_ADDR 0x10

//sw2001 aes control addr
#define SW2001_REG_DECRYPT_CTRL 0x00

#define SW2001_AES_ENABLE (0x01)

#define SW2001_AES_STATUS (0x20) 
/**
 * \brief          AES context structure
 */
typedef struct
{
    int32_t nr;                     /*!<  number of rounds  */
    uint32_t *rk;               /*!<  AES round keys    */
    uint32_t buf[68];           /*!<  unaligned data    */
}aes_context;

uint32_t sw2001_aes_parity(uint8_t *text, uint8_t *parity);
uint32_t sw2001_aes_compute(uint8_t *key, uint8_t *pt, uint8_t *ct);
void aes_gen_tables( void );
uint32_t aes_setkey_enc( aes_context *ctx, const uint8_t *key, uint32_t keysize );
uint32_t aes_crypt_ecb( aes_context *ctx,const uint8_t input[16],uint8_t output[16] );

#endif