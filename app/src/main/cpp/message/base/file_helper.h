#ifndef FILE_HELPER_H_
#define FILE_HELPER_H_

#include <hpsocket/common/GlobalDef.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "base/public_def.h"

#define INVALID_MAP_ADDR	((PBYTE)(MAP_FAILED))

std::string GetCurrentDirectory();
std::string GetModuleFileName(pid_t pid = 0);
BOOL SetCurrentPathToModulePath(pid_t pid = 0);

class CFile
{
public:
	BOOL Open(LPCTSTR lpszFilePath, int iFlag, mode_t iMode = 0);
	BOOL Close();
	BOOL Stat(struct stat& st);
	BOOL GetSize(SIZE_T& dwSize);

	SSIZE_T Read(PVOID pBuffer, SIZE_T dwCount)
		{return read(m_fd, pBuffer, dwCount);}
	SSIZE_T Write(PVOID pBuffer, SIZE_T dwCount)
		{return write(m_fd, pBuffer, dwCount);}
	SSIZE_T PRead(PVOID pBuffer, SIZE_T dwCount, SIZE_T dwOffset)
		{return pread(m_fd, pBuffer, dwCount, dwOffset);}
	SSIZE_T PWrite(PVOID pBuffer, SIZE_T dwCount, SIZE_T dwOffset)
		{return pwrite(m_fd, pBuffer, dwCount, dwOffset);}
	SSIZE_T ReadV(const iovec* pVec, int iVecCount)
		{return readv(m_fd, pVec, iVecCount);}
	SSIZE_T WriteV(const iovec* pVec, int iVecCount)
		{return writev(m_fd, pVec, iVecCount);}
	SSIZE_T Seek(SSIZE_T lOffset, int iWhence)
		{return lseek(m_fd, lOffset, iWhence);}

	BOOL IsValid()	{return m_fd != -1;}
	operator FD ()	{return m_fd;}

	BOOL IsExist()	{return IsValid();}

	BOOL IsDirectory();
	BOOL IsFile();

	static BOOL IsExist(LPCTSTR lpszFilePath);
	static BOOL IsDirectory(LPCTSTR lpszFilePath);
	static BOOL IsFile(LPCTSTR lpszFilePath);
	static BOOL IsLink(LPCTSTR lpszFilePath);

public:
	CFile(LPCTSTR lpszFilePath = nullptr, int iFlag = O_RDONLY, mode_t iMode = 0)
	: m_fd(INVALID_FD)
	{
		if(lpszFilePath != nullptr)
			Open(lpszFilePath, iFlag, iMode);
	}

	~CFile()
	{
		if(IsValid())
			Close();
	}

private:
	FD m_fd;
};

class CFileMapping
{
public:
	BOOL Map(LPCTSTR lpszFilePath, SIZE_T dwSize = 0, SIZE_T dwOffset = 0, int iProtected = PROT_READ, int iFlag = MAP_PRIVATE);
	BOOL Map(FD fd, SIZE_T dwSize = 0, SIZE_T dwOffset = 0, int iProtected = PROT_READ, int iFlag = MAP_PRIVATE);
	BOOL Unmap();
	BOOL MSync(int iFlag = MS_SYNC, SIZE_T dwSize = 0);

	BOOL IsValid	()		{return m_pv != INVALID_MAP_ADDR;}
	SIZE_T Size		()		{return m_dwSize;}
	LPBYTE Ptr		()		{return m_pv;}
	operator LPBYTE	()		{return Ptr();}

public:
	CFileMapping()
	: m_pv(INVALID_MAP_ADDR)
	, m_dwSize(0)
	{

	}

	~CFileMapping()
	{
		if(IsValid())
			Unmap();
	}

private:
	PBYTE	m_pv;
	SIZE_T	m_dwSize;
};

#endif