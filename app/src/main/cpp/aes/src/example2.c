#include "interface.h"
#include "sw200x_aes.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <jni.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>



void test_example()
{
	uint8_t pt[16] ={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
	uint8_t ct[16]; 
	uint8_t result[16];
	uint8_t parity=0;
	uint8_t i;
	
	//init I2C, used to acess sw200x
	iDeviceInit(SW200x_DEVICE_ADDR, 0);   //sw2001 default device address=0x3C, speed support 100k/400k

	//generate random plait text
	//srand( (unsigned)time( NULL ) );
	for(i=0;i<16;i++)
	{
		pt[i] = rand()%256;
	}

	//compute cipher text
	sw200x_aes_compute(key, pt, ct);
	
	//compute odd check bits
	sw200x_aes_parity(pt, &parity);

	//write cipher text to sw200x
	iWriteData(SW200x_REG_CIPHER_TEXT_ADDR, ct, 16);
	
	//set parity
	iWriteByte(SW200x_REG_DECRYPT_CTRL,parity);

	//start sw200x aes decrypt
	iSetBits(SW200x_REG_DECRYPT_CTRL, SW200x_AES_ENABLE);

	//wait for sw200x decrypt complete
	iCheckBits(SW200x_REG_DECRYPT_CTRL, SW200x_AES_STATUS, 0x0);

	//read decrypt result back
	iReadData(SW200x_REG_PLAINT_TEXT_ADDR, result, 16);
	
	
	//compare result
	for(i=0;i<16;i++)
	{
		if(pt[i] != result[i])
		{
			printf("verify failed---------------------------------!\n");
			return;
		}
	}
	
	printf("verify success!\n");
	
	//deinit I2C, release resource
	iDeviceDeInit();
	
	return;
}
