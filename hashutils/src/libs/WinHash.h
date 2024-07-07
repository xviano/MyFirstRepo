/**
 * Windows Hashing/Checksumming Library
 * Last modified: 2009/03/04
 * Copyright (C) Kai Liu.  All rights reserved.
 *
 * This is a wrapper for the MD4, MD5, SHA1, and CRC32 algorithms supported
 * natively by the Windows API.
 **/

#ifndef __WINHASH_H__
#define __WINHASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include "SwapIntrinsics.h"


/*******************************************************************************
 *
 *
 * Helper definitions
 *
 *
 ******************************************************************************/


typedef CONST BYTE *PCBYTE;

#define WHAPI __fastcall

#ifndef WH_NO_DLL_IMPORT
#define WHDLLIMP __declspec( dllimport )
#else
#define WHDLLIMP
#endif


/*******************************************************************************
 *
 *
 * Functions to copy byte arrays
 *
 *
 ******************************************************************************/


__forceinline VOID WHCopy32Bits( PBYTE pbDest, PCBYTE pbSrc )
{
	*(UINT32 UNALIGNED *)pbDest = *(UINT32 UNALIGNED *)pbSrc;
}

#if defined(__MOVS_STOS_DEFINED)

#define WHCopy128Bits(pbDest, pbSrc) __movsd((PVOID)pbDest, (PVOID)pbSrc, 4)
#define WHCopy160Bits(pbDest, pbSrc) __movsd((PVOID)pbDest, (PVOID)pbSrc, 5)

#else

#pragma intrinsic(memcpy)
#define WHCopy128Bits(pbDest, pbSrc) memcpy(pbDest, pbSrc, 16)
#define WHCopy160Bits(pbDest, pbSrc) memcpy(pbDest, pbSrc, 20)

#endif


/*******************************************************************************
 *
 *
 * Structures used by the system libraries
 *
 *
 ******************************************************************************/


typedef struct {
	UINT32 state[4];
	UINT32 count[2];
	BYTE buffer[64];
	BYTE result[16];
} MD4_CTX, *PMD4_CTX, MD5_CTX, *PMD5_CTX;

typedef struct {
	UINT32 reserved[6];
	UINT32 state[5];
	UINT32 count[2];
	BYTE buffer[64];
} SHA1_CTX, *PSHA1_CTX;


/*******************************************************************************
 *
 *
 * Functions imported from system libraries
 *
 *
 ******************************************************************************/


WHDLLIMP UINT32 WINAPI RtlComputeCrc32( UINT32 uInitial, PCBYTE pbIn, UINT cbIn );

WHDLLIMP VOID WINAPI MD4Init( PMD4_CTX pContext );
WHDLLIMP VOID WINAPI MD4Update( PMD4_CTX pContext, PCBYTE pbIn, UINT cbIn );
WHDLLIMP VOID WINAPI MD4Final( PMD4_CTX pContext );

WHDLLIMP VOID WINAPI MD5Init( PMD5_CTX pContext );
WHDLLIMP VOID WINAPI MD5Update( PMD5_CTX pContext, PCBYTE pbIn, UINT cbIn );
WHDLLIMP VOID WINAPI MD5Final( PMD5_CTX pContext );

WHDLLIMP VOID WINAPI A_SHAInit( PSHA1_CTX pContext );
WHDLLIMP VOID WINAPI A_SHAUpdate( PSHA1_CTX pContext, PCBYTE pbIn, UINT cbIn );
WHDLLIMP VOID WINAPI A_SHAFinal( PSHA1_CTX pContext, PBYTE pbResult );


/*******************************************************************************
 *
 *
 * Wrapper layer: CRC-32
 *
 *
 ******************************************************************************/


typedef union {
	UINT32 state;
	BYTE result[4];
} WHCTXCRC32, *PWHCTXCRC32;

__forceinline VOID WHInitCRC32( PWHCTXCRC32 pContext )
{
	pContext->state = 0;
}

__forceinline VOID WHUpdateCRC32( PWHCTXCRC32 pContext, PCBYTE pbIn, UINT cbIn )
{
	pContext->state = RtlComputeCrc32(pContext->state, pbIn, cbIn);
}

__forceinline VOID WHFinishCRC32( PWHCTXCRC32 pContext )
{
	pContext->state = SwapV32(pContext->state);
}

__forceinline VOID WHFinishCRC32Ex( PWHCTXCRC32 pContext, PBYTE pbResult )
{
	*(UINT32 UNALIGNED *)pbResult = SwapV32(pContext->state);
}


/*******************************************************************************
 *
 *
 * Wrapper layer: MD4
 *
 *
 ******************************************************************************/


#define  WHCTXMD4  MD4_CTX
#define PWHCTXMD4 PMD4_CTX

#define WHInitMD4 MD4Init
#define WHUpdateMD4 MD4Update
#define WHFinishMD4 MD4Final

__forceinline VOID WHFinishMD4Ex( PWHCTXMD4 pContext, PBYTE pbResult )
{
	MD4Final(pContext);
	WHCopy128Bits(pbResult, pContext->result);
}


/*******************************************************************************
 *
 *
 * Wrapper layer: MD5
 *
 *
 ******************************************************************************/


#define  WHCTXMD5  MD5_CTX
#define PWHCTXMD5 PMD5_CTX

#define WHInitMD5 MD5Init
#define WHUpdateMD5 MD5Update
#define WHFinishMD5 MD5Final

__forceinline VOID WHFinishMD5Ex( PWHCTXMD5 pContext, PBYTE pbResult )
{
	MD5Final(pContext);
	WHCopy128Bits(pbResult, pContext->result);
}


/*******************************************************************************
 *
 *
 * Wrapper layer: SHA-1
 *
 *
 ******************************************************************************/


typedef struct {
	SHA1_CTX ctx;
	BYTE result[20];
} WHCTXSHA1, *PWHCTXSHA1;

__forceinline VOID WHInitSHA1( PWHCTXSHA1 pContext )
{
	A_SHAInit(&pContext->ctx);
}

__forceinline VOID WHUpdateSHA1( PWHCTXSHA1 pContext, PCBYTE pbIn, UINT cbIn )
{
	A_SHAUpdate(&pContext->ctx, pbIn, cbIn);
}

__forceinline VOID WHFinishSHA1( PWHCTXSHA1 pContext )
{
	A_SHAFinal(&pContext->ctx, pContext->result);
}

__forceinline VOID WHFinishSHA1Ex( PWHCTXSHA1 pContext, PBYTE pbResult )
{
	A_SHAFinal(&pContext->ctx, pbResult);
}


/*******************************************************************************
 *
 *
 * Wrapper layer: ED2K
 *
 *
 ******************************************************************************/


typedef struct {
	MD4_CTX ctxList;
	MD4_CTX ctxChunk;
	PBYTE result;
	UINT cbChunkRemaining;
} WHCTXED2K, *PWHCTXED2K;

__forceinline VOID WHInitED2K( PWHCTXED2K pContext )
{
	MD4Init(&pContext->ctxList);
	MD4Init(&pContext->ctxChunk);
	pContext->cbChunkRemaining = 9500 << 10;
	pContext->result = pContext->ctxChunk.result;
}

__forceinline VOID WHUpdateED2K( PWHCTXED2K pContext, PCBYTE pbIn, UINT cbIn )
{
	if (cbIn >= pContext->cbChunkRemaining)
	{
		// Finish off the current chunk and add it to the list hash
		MD4Update(&pContext->ctxChunk, pbIn, pContext->cbChunkRemaining);
		MD4Final(&pContext->ctxChunk);
		MD4Update(&pContext->ctxList, pContext->ctxChunk.result, sizeof(pContext->ctxChunk.result));
		pbIn += pContext->cbChunkRemaining;
		cbIn -= pContext->cbChunkRemaining;

		// Reset the chunk context
		MD4Init(&pContext->ctxChunk);
		pContext->cbChunkRemaining = 9500 << 10;

		// The final result will now be the list hash, not the chunk hash
		pContext->result = pContext->ctxList.result;
	}

	MD4Update(&pContext->ctxChunk, pbIn, cbIn);
	pContext->cbChunkRemaining -= cbIn;
}

__forceinline VOID WHFinishED2K( PWHCTXED2K pContext )
{
	MD4Final(&pContext->ctxChunk);
	MD4Update(&pContext->ctxList, pContext->ctxChunk.result, sizeof(pContext->ctxChunk.result));
	MD4Final(&pContext->ctxList);
}

__forceinline VOID WHFinishED2KEx( PWHCTXED2K pContext, PBYTE pbResult )
{
	WHFinishED2K(pContext);
	WHCopy128Bits(pbResult, pContext->result);
}


/*******************************************************************************
 *
 *
 * WH*To* hex string conversion functions: These require WinHash.c
 *
 *
 ******************************************************************************/


#define WHFMT_UPPERCASE 0x00
#define WHFMT_LOWERCASE 0x20

BOOL WHAPI WHHexToByte( PCTSTR pszSrc, PBYTE pbDest, UINT cchHex );
PTSTR WHAPI WHByteToHex( PCBYTE pbSrc, PTSTR pszDest, UINT cchHex, UINT8 uCaseMode );


/*******************************************************************************
 *
 *
 * WH*Ex functions: These require WinHash.c
 *
 *
 ******************************************************************************/


typedef struct {
	BYTE CRC32[4];
	BYTE MD4[16];
	BYTE MD5[16];
	BYTE SHA1[20];
	BYTE ED2K[16];
} WHRESULTEX, *PWHRESULTEX;

typedef struct {
	UINT8      fAlgs;
	UINT8      uCaseMode;
	WHCTXCRC32 ctxCRC32;
	WHCTXMD4   ctxMD4;
	WHCTXMD5   ctxMD5;
	WHCTXSHA1  ctxSHA1;
	WHCTXED2K  ctxED2K;
	WHRESULTEX results;
} WHCTXEX, *PWHCTXEX;

#define WHEX_CHECKCRC32 0x01
#define WHEX_CHECKMD4   0x02
#define WHEX_CHECKMD5   0x04
#define WHEX_CHECKSHA1  0x08
#define WHEX_CHECKED2K  0x10
#define WHEX_ALL        0x1F
#define WHEX_ALL32      0x01
#define WHEX_ALL128     0x16
#define WHEX_ALL160     0x08

VOID WHAPI WHInitEx( PWHCTXEX pContext );
VOID WHAPI WHUpdateEx( PWHCTXEX pContext, PCBYTE pbIn, UINT cbIn );
VOID WHAPI WHFinishEx( PWHCTXEX pContext, PWHRESULTEX pResults );


#ifdef __cplusplus
}
#endif

#endif
