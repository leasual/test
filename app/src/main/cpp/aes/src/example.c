#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "interface.h"
#include "sw2001_aes.h"


void main()
{
	uint8_t pt[16] ={0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d,0x31,0x31,0x98,0xa2,0xe0,0x37,0x07,0x34};
	uint8_t ct[16]; // = {0x39,0x25,0x84,0x1d,0x02,0xdc,0x09,0xfb,0xdc,0x11,0x85,0x97,0x19,0x6a,0x0b,0x32};
	uint8_t key[16] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c}; //user define according to sw2001 ic
	uint8_t result[16];
	uint8_t parity=0;
	int32 i;
	
	//init I2C, used to acess sw2001
	iDeviceInit(SW2001_DEVICE_ADDR, 0);   //sw2001 default device address=0x3C, speed support 100k/400k

	//generate random plait text
	srand( (unsigned)time( NULL ) );
	for(i=0;i<16;i++)
	{
		pt[i] = rand()%256;
	}

	//compute cipher text
	sw2001_aes_compute(key, pt, ct);
	
	//compute odd check bits
	sw2001_aes_parity(pt, &parity);

	//write cipher text to sw2001
//	iWriteData(SW2001_REG_CIPHER_TEXT_ADDR, ct, 16);
	
	//set parity
	iWriteByte(SW2001_REG_DECRYPT_CTRL,parity);

	//start sw2001 aes decrypt
//	iSetBits(SW2001_REG_DECRYPT_CTRL, SW2001_AES_ENABLE);

	//wait for sw2001 decrypt complete
//	iCheckBits(SW2001_REG_DECRYPT_CTRL, SW2001_AES_STATUS, 0x0);

	//read decrypt result back
//	iReadData(SW2001_REG_PLAINT_TEXT_ADDR, result, 16);

	//compare result
	for(i=0;i<16;i++)
	{
		if(pt[i] != result[i])
		{
			printf("verify failed!\n");
			return;
		}
	}
	printf("verify success!\n");
	return;
}
