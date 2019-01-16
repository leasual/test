//*****************************************************************************
//	File name  : 				sw2001_aes.c
//
//	Description:  This file provides all functions of aes
//	
//  written by :  jason, iSmartware Technology Co.,LTD
//	History    :
//                 2015/04/13      jason       v0.1    Initial version
//                 2015/4/24       jason       v0.2    added parity check
//******************************************************************************
#include "sw2001_aes.h"
static int32_t aes_init_done = 0;

//*********************************************************************************
// function    : sw2001_aes_parity(uint8_t *text, uint8_t *parity)
// description : compute parity bit of plain text
// parameters  :
//                text  : aes plain text
//                parity  : parity of plain text
// return        : 
//               TRUE : success
//               FALSE : fail
//*********************************************************************************
uint32_t sw2001_aes_parity(uint8_t *text, uint8_t *parity)
{
	uint32_t *textpt = (__u32 *)text;
	uint32_t temp=0, temp1=0;
	uint32_t i=0;
	for(i=0;i<4;i++)
	{
		temp = *textpt++;
		temp ^= temp>>16;
		temp ^= temp>>8;
		temp ^= temp>>4;
		temp ^= temp>>2;
		temp ^= temp>>1;
		temp1 |= (temp & 0x1) <<(4-i);
	}
	*parity = temp1;
	return (0);
}

//*********************************************************************************
// function    : sw2001_aes_compute(uint8_t *key, uint8_t *pt, uint8_t *ct)
// description : compute the cipher text according plain text and key
// parameters  :
//                key : aes 128 bit key
//                pt  : aes plain text
//                ct  : aes cipher text
// return        : 
//               TRUE : success
//               FALSE : fail
//*********************************************************************************
uint32_t sw2001_aes_compute(uint8_t *key, uint8_t *pt, uint8_t *ct)
{
	uint8_t bufin[16];
	uint8_t bufout[16];
	aes_context ctx;

	if(aes_init_done==0)
	{
		aes_gen_tables();
	}
	memcpy(bufin,pt,16);
	aes_setkey_enc( &ctx, key, 128 );
	aes_crypt_ecb( &ctx, bufin, bufout );  
	memcpy(ct, bufout, 16);
	return (0);
}

/*
 * 32-bit integer manipulation macros (little endian)
 */
#ifndef GET_UINT32_LE
#define GET_UINT32_LE(n,b,i)                            \
{                                                       \
    (n) = ( (uint32_t) (b)[(i)    ]       )             \
        | ( (uint32_t) (b)[(i) + 1] <<  8 )             \
        | ( (uint32_t) (b)[(i) + 2] << 16 )             \
        | ( (uint32_t) (b)[(i) + 3] << 24 );            \
}
#endif

#ifndef PUT_UINT32_LE
#define PUT_UINT32_LE(n,b,i)                            \
{                                                       \
    (b)[(i)    ] = (uint8_t) ( (n)       );       \
    (b)[(i) + 1] = (uint8_t) ( (n) >>  8 );       \
    (b)[(i) + 2] = (uint8_t) ( (n) >> 16 );       \
    (b)[(i) + 3] = (uint8_t) ( (n) >> 24 );       \
}
#endif

#define AES_FROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)     \
{                                               \
    X0 = *RK++ ^ FT0[ ( Y0       ) & 0xFF ] ^   \
                 FT1[ ( Y1 >>  8 ) & 0xFF ] ^   \
                 FT2[ ( Y2 >> 16 ) & 0xFF ] ^   \
                 FT3[ ( Y3 >> 24 ) & 0xFF ];    \
                                                \
    X1 = *RK++ ^ FT0[ ( Y1       ) & 0xFF ] ^   \
                 FT1[ ( Y2 >>  8 ) & 0xFF ] ^   \
                 FT2[ ( Y3 >> 16 ) & 0xFF ] ^   \
                 FT3[ ( Y0 >> 24 ) & 0xFF ];    \
                                                \
    X2 = *RK++ ^ FT0[ ( Y2       ) & 0xFF ] ^   \
                 FT1[ ( Y3 >>  8 ) & 0xFF ] ^   \
                 FT2[ ( Y0 >> 16 ) & 0xFF ] ^   \
                 FT3[ ( Y1 >> 24 ) & 0xFF ];    \
                                                \
    X3 = *RK++ ^ FT0[ ( Y3       ) & 0xFF ] ^   \
                 FT1[ ( Y0 >>  8 ) & 0xFF ] ^   \
                 FT2[ ( Y1 >> 16 ) & 0xFF ] ^   \
                 FT3[ ( Y2 >> 24 ) & 0xFF ];    \
}



/*
 * Forward S-box & tables
 */
static uint8_t FSb[256];
static uint32_t FT0[256]; 
static uint32_t FT1[256]; 
static uint32_t FT2[256]; 
static uint32_t FT3[256]; 

/*
 * Round constants
 */
static uint32_t RCON[10];

/*
 * Tables generation code
 */
#define ROTL8(x) ( ( x << 8 ) & 0xFFFFFFFF ) | ( x >> 24 )
#define XTIME(x) ( ( x << 1 ) ^ ( ( x & 0x80 ) ? 0x1B : 0x00 ) )
#define MUL(x,y) ( ( x && y ) ? pow[(log[x]+log[y]) % 255] : 0 )

void aes_gen_tables( void )
{
    int32_t i, x, y, z;
    int32_t pow[256];
    int32_t log[256];

    /*
     * compute pow and log tables over GF(2^8)
     */
    for( i = 0, x = 1; i < 256; i++ )
    {
        pow[i] = x;
        log[x] = i;
        x = ( x ^ XTIME( x ) ) & 0xFF;
    }

    /*
     * calculate the round constants
     */
    for( i = 0, x = 1; i < 10; i++ )
    {
        RCON[i] = (uint32_t) x;
        x = XTIME( x ) & 0xFF;
    }

    /*
     * generate the forward S-boxes
     */
    FSb[0x00] = 0x63;

    for( i = 1; i < 256; i++ )
    {
        x = pow[255 - log[i]];

        y  = x; y = ( (y << 1) | (y >> 7) ) & 0xFF;
        x ^= y; y = ( (y << 1) | (y >> 7) ) & 0xFF;
        x ^= y; y = ( (y << 1) | (y >> 7) ) & 0xFF;
        x ^= y; y = ( (y << 1) | (y >> 7) ) & 0xFF;
        x ^= y ^ 0x63;

        FSb[i] = (uint8_t) x;
    }

    /*
     * generate the forwardtables
     */
    for( i = 0; i < 256; i++ )
    {
        x = FSb[i];
        y = XTIME( x ) & 0xFF;
        z =  ( y ^ x ) & 0xFF;

        FT0[i] = ( (uint32_t) y       ) ^
                 ( (uint32_t) x <<  8 ) ^
                 ( (uint32_t) x << 16 ) ^
                 ( (uint32_t) z << 24 );

        FT1[i] = ROTL8( FT0[i] );
        FT2[i] = ROTL8( FT1[i] );
        FT3[i] = ROTL8( FT2[i] );
    }
}


/*
 * AES key schedule (encryption)
 */
uint32_t aes_setkey_enc( aes_context *ctx, const uint8_t *key, uint32_t keysize )
{
    uint32_t i;
    uint32_t *RK;

    ctx->nr = 10;

    ctx->rk = RK = ctx->buf;

    for( i = 0; i < (keysize >> 5); i++ )
    {
        GET_UINT32_LE( RK[i], key, i << 2 );
    }

    for( i = 0; i < 10; i++, RK += 4 )
    {
        RK[4]  = RK[0] ^ RCON[i] ^
        ( (uint32_t) FSb[ ( RK[3] >>  8 ) & 0xFF ]       ) ^
        ( (uint32_t) FSb[ ( RK[3] >> 16 ) & 0xFF ] <<  8 ) ^
        ( (uint32_t) FSb[ ( RK[3] >> 24 ) & 0xFF ] << 16 ) ^
        ( (uint32_t) FSb[ ( RK[3]       ) & 0xFF ] << 24 );

        RK[5]  = RK[1] ^ RK[4];
        RK[6]  = RK[2] ^ RK[5];
        RK[7]  = RK[3] ^ RK[6];
    }

    return (0);
}

/*
 * AES-ECB block encryption
 */
uint32_t aes_crypt_ecb( aes_context *ctx,const uint8_t input[16],uint8_t output[16] )
{
    int i;
    uint32_t *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

    RK = ctx->rk;

    GET_UINT32_LE( X0, input,  0 ); X0 ^= *RK++;
    GET_UINT32_LE( X1, input,  4 ); X1 ^= *RK++;
    GET_UINT32_LE( X2, input,  8 ); X2 ^= *RK++;
    GET_UINT32_LE( X3, input, 12 ); X3 ^= *RK++;

    for( i = (ctx->nr >> 1) - 1; i > 0; i-- )
    {
        AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );
        AES_FROUND( X0, X1, X2, X3, Y0, Y1, Y2, Y3 );
    }

    AES_FROUND( Y0, Y1, Y2, Y3, X0, X1, X2, X3 );

    X0 = *RK++ ^ \
            ( (uint32_t) FSb[ ( Y0       ) & 0xFF ]       ) ^
            ( (uint32_t) FSb[ ( Y1 >>  8 ) & 0xFF ] <<  8 ) ^
            ( (uint32_t) FSb[ ( Y2 >> 16 ) & 0xFF ] << 16 ) ^
            ( (uint32_t) FSb[ ( Y3 >> 24 ) & 0xFF ] << 24 );

    X1 = *RK++ ^ \
            ( (uint32_t) FSb[ ( Y1       ) & 0xFF ]       ) ^
            ( (uint32_t) FSb[ ( Y2 >>  8 ) & 0xFF ] <<  8 ) ^
            ( (uint32_t) FSb[ ( Y3 >> 16 ) & 0xFF ] << 16 ) ^
            ( (uint32_t) FSb[ ( Y0 >> 24 ) & 0xFF ] << 24 );

    X2 = *RK++ ^ \
            ( (uint32_t) FSb[ ( Y2       ) & 0xFF ]       ) ^
            ( (uint32_t) FSb[ ( Y3 >>  8 ) & 0xFF ] <<  8 ) ^
            ( (uint32_t) FSb[ ( Y0 >> 16 ) & 0xFF ] << 16 ) ^
            ( (uint32_t) FSb[ ( Y1 >> 24 ) & 0xFF ] << 24 );

    X3 = *RK++ ^ \
            ( (uint32_t) FSb[ ( Y3       ) & 0xFF ]       ) ^
            ( (uint32_t) FSb[ ( Y0 >>  8 ) & 0xFF ] <<  8 ) ^
            ( (uint32_t) FSb[ ( Y1 >> 16 ) & 0xFF ] << 16 ) ^
            ( (uint32_t) FSb[ ( Y2 >> 24 ) & 0xFF ] << 24 );
    

    PUT_UINT32_LE( X0, output,  0 );
    PUT_UINT32_LE( X1, output,  4 );
    PUT_UINT32_LE( X2, output,  8 );
    PUT_UINT32_LE( X3, output, 12 );

    return (0);
}

