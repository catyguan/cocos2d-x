#include "CCFileSystemWin32.h"

NS_CC_BEGIN

CCFileSystemWin32::CCFileSystemWin32()
{
}

CCFileSystemWin32::~CCFileSystemWin32()
{
}

bool CCFileSystemWin32::fileExists(const char* pszFileName)
{
	DWORD fa = ::GetFileAttributesA(pszFileName);
	return fa != INVALID_FILE_ATTRIBUTES;
}

unsigned char* CCFileSystemWin32::fileRead(const char* pszFileName, unsigned long* pSize)
{
	int size = 0;
	unsigned char* buf = NULL;
	do {
		// read the file from hardware
		FILE *fp = fopen(pszFileName, "rb");
		CC_BREAK_IF(!fp);
        
		fseek(fp,0,SEEK_END);
		size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		buf = new unsigned char[size];
		fread(buf,sizeof(char), size,fp);
		fclose(fp);
    } while (0);
    
    if (! buf)
    {
        return NULL;
    }    
	*pSize = size;
	return buf;
}

unsigned long CCFileSystemWin32::fileWrite(const char* pszFileName, unsigned char* content, long size)
{
	FILE *fp = fopen(pszFileName, "wb");
	bool r = false;
	size_t wcount = 0;
	if(fp) {
		wcount = fwrite(content, size,1, fp);
		fclose(fp);	
	}
	return wcount;
}

bool CCFileSystemWin32::fileDelete(const char* pszFileName)
{
	return ::DeleteFileA(pszFileName)==TRUE;
}

NS_CC_END

