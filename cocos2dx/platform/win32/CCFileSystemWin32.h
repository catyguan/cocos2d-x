#ifndef __CC_FILESYSTEM_WIN32_H__
#define __CC_FILESYSTEM_WIN32_H__

#include "../CCFileSystem.h"

NS_CC_BEGIN

class CC_DLL CCFileSystemWin32 : public CCFileSystemBase
{
public:   
	CCFileSystemWin32();
    virtual ~CCFileSystemWin32();

protected:
	virtual bool fileExists(const char* pszFileName);
	virtual unsigned char* fileRead(const char* pszFileName, unsigned long* pSize);
	virtual unsigned long fileWrite(const char* pszFileName, unsigned char* content, long size);
	virtual bool fileDelete(const char* pszFileName);
};

NS_CC_END

#endif    // __CC_FILESYSTEM_PROTOCOL_H__
