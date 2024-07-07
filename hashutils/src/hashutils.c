#pragma comment(linker, "/version:1.3") // MUST be in the form of major.minor

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "libs\GetArgv.h"
#include "libs\SimpleString.h"
#include "libs\WinHash.h"

// A large buffer will reduce the number of read operations and improve
// performance by reducing overhead, but there seems to be little or no gain
// beyond 0x40000 bytes.
#ifndef READ_BUFFER_SIZE
#define READ_BUFFER_SIZE 0x40000
#endif

void __stdcall DoHash( PCTSTR pszSearchPath, PVOID pvBuffer, BOOL fRecurse );
__forceinline void PrintInt( PVOID pvResult, PCTSTR pszPath );
__forceinline void PrintArr( PVOID pvResult, PCTSTR pszPath, SIZE_T cdwResult );

#define WHPrintCRC32      PrintInt
#define WHPrintMD4(a, b)  PrintArr(a, b, 4)
#define WHPrintMD5(a, b)  PrintArr(a, b, 4)
#define WHPrintSHA1(a, b) PrintArr(a, b, 5)
#define WHPrintED2K(a, b) PrintArr(a, b, 4)

#if defined(WHTYPE_CRC32)
#define WHNAME   "crc32sum"
#define WHCTX    WHCTXCRC32
#define WHInit   WHInitCRC32
#define WHUpdate WHUpdateCRC32
#define WHFinish WHFinishCRC32
#define WHPrint  WHPrintCRC32
#elif defined(WHTYPE_MD4)
#define WHNAME   "md4sum"
#define WHCTX    WHCTXMD4
#define WHInit   WHInitMD4
#define WHUpdate WHUpdateMD4
#define WHFinish WHFinishMD4
#define WHPrint  WHPrintMD4
#elif defined(WHTYPE_MD5)
#define WHNAME   "md5sum"
#define WHCTX    WHCTXMD5
#define WHInit   WHInitMD5
#define WHUpdate WHUpdateMD5
#define WHFinish WHFinishMD5
#define WHPrint  WHPrintMD5
#elif defined(WHTYPE_SHA1)
#define WHNAME   "sha1sum"
#define WHCTX    WHCTXSHA1
#define WHInit   WHInitSHA1
#define WHUpdate WHUpdateSHA1
#define WHFinish WHFinishSHA1
#define WHPrint  WHPrintSHA1
#elif defined(WHTYPE_ED2K)
#define WHNAME   "ed2ksum"
#define WHCTX    WHCTXED2K
#define WHInit   WHInitED2K
#define WHUpdate WHUpdateED2K
#define WHFinish WHFinishED2K
#define WHPrint  WHPrintED2K
#else
#error Invalid WHTYPE
#endif

__forceinline BOOL CheckFlag( PCTSTR pszArg, TCHAR ch );
__forceinline BOOL CheckFlagI( PCTSTR pszArg, TCHAR ch );
__forceinline BOOL IsSpecialDirectoryName( PCTSTR pszPath );

#pragma comment(linker, "/entry:hashutils")
void hashutils( )
{
	UINT uRetCode = 1;

	// Allocate the buffer outside of DoHash so that DoHash can use the same
	// buffer with each recursive call.
	PVOID pvBuffer = LocalAlloc(LMEM_FIXED, READ_BUFFER_SIZE);

	UINT argc = 0;
	PTSTR *argv = GetArgv(&argc);

	if (pvBuffer && argv)
	{
		BOOL fRecurse = FALSE;
		UINT i;

		// Check flags
		for (i = 1; i < argc; ++i)
		{
			if (CheckFlag(argv[i], TEXT('?')))
				goto usage;
			else if (CheckFlagI(argv[i], TEXT('r')))
				fRecurse = TRUE;
			else
				break;
		}

		// Process the rest of the tokens
		for (; i < argc; ++i)
		{
			DoHash(argv[i], pvBuffer, fRecurse);
			uRetCode = 0;
		}

		usage: if (uRetCode)
			_tprintf(TEXT("Usage: ") TEXT(WHNAME) TEXT(" [-r] filenames\n"));
	}

	LocalFree(argv);
	LocalFree(pvBuffer);
	ExitProcess(uRetCode);
}

void __stdcall DoHash( PCTSTR pszSearchPath, PVOID pvBuffer, BOOL fRecurse )
{
	HANDLE hFind;
	WIN32_FIND_DATA finddata;

	// Copy the path to a scratchpad buffer and establish the append position
	TCHAR szPath[MAX_PATH << 2];
	PTSTR pszPathAppend;

	lstrcpy(szPath, pszSearchPath);

	if (pszPathAppend = _tcsrchr(szPath, TEXT('\\')))
		++pszPathAppend;
	else
		pszPathAppend = szPath;

	if ((hFind = FindFirstFile(pszSearchPath, &finddata)) != INVALID_HANDLE_VALUE)
	{
		HANDLE hFile;
		WHCTX ctx;
		DWORD cbBytesRead;

		do
		{
			if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				lstrcpy(pszPathAppend, finddata.cFileName);

				WHInit(&ctx);

				hFile = CreateFile(
					szPath,
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
					NULL
				);

				if (hFile != INVALID_HANDLE_VALUE)
				{
					do
					{
						ReadFile(hFile, pvBuffer, READ_BUFFER_SIZE, &cbBytesRead, NULL);
						WHUpdate(&ctx, pvBuffer, cbBytesRead);

					} while (cbBytesRead == READ_BUFFER_SIZE);

					CloseHandle(hFile);
				}

				WHFinish(&ctx);
				WHPrint(ctx.result, szPath);
			}
			else if (fRecurse && !IsSpecialDirectoryName(finddata.cFileName))
			{
				if (IsSpecialDirectoryName(pszSearchPath))
				{
					// If the user explicitly specified "." or  "..",
					// FindFirstFile will return the actual directory name,
					// which is stupid because it loses the path relation.

					lstrcpy(pszPathAppend, pszSearchPath);
				}
				else
				{
					lstrcpy(pszPathAppend, finddata.cFileName);
				}

				lstrcat(pszPathAppend, TEXT("\\*"));
				DoHash(szPath, pvBuffer, fRecurse);
			}

		} while (FindNextFile(hFind, &finddata));

		FindClose(hFind);
	}
}

__forceinline void PrintInt( PVOID pvResult, PCTSTR pszPath )
{
	PDWORD pdwResult = pvResult;

	_tprintf(TEXT("%-12s %08x\n"), pszPath, SwapV32(*pdwResult));
}

__forceinline void PrintArr( PVOID pvResult, PCTSTR pszPath, SIZE_T cdwResult )
{
	PDWORD pdwResult = pvResult;

	while (cdwResult)
	{
		_tprintf(TEXT("%08x"), SwapV32(*pdwResult));
		++pdwResult;
		--cdwResult;
	}

	_tprintf(TEXT(" *%s\n"), pszPath);
}

__forceinline BOOL CheckFlag( PCTSTR pszArg, TCHAR ch )
{
	// These checks must be done piece-meal with no WORD/DWORD/memcmp shortcuts
	// because there is no guarantee that the pszArg is large enough
	return(
		(pszArg[0] | 0x02) == TEXT('/') &&
		(pszArg[1]       ) == ch &&
		(pszArg[2]       ) == 0
	);
}

__forceinline BOOL CheckFlagI( PCTSTR pszArg, TCHAR ch )
{
	return(
		(pszArg[0] | 0x02) == TEXT('/') &&
		(pszArg[1] | 0x20) == ch &&
		(pszArg[2]       ) == 0
	);
}

__forceinline BOOL IsSpecialDirectoryName( PCTSTR pszPath )
{
	// TRUE if name is "." or ".."

	#ifdef UNICODE
	return(
		(*((UPDWORD)pszPath) == WCHARS2DWORD(L'.', 0)) ||
		(*((UPDWORD)pszPath) == WCHARS2DWORD(L'.', L'.') && pszPath[2] == 0)
	);
	#else
	return(
		(*((UPWORD)pszPath) == CHARS2WORD('.', 0)) ||
		(*((UPWORD)pszPath) == CHARS2WORD('.', '.') && pszPath[2] == 0)
	);
	#endif
}
