#ifndef __CC_FILESYSTEM_PROTOCOL_H__
#define __CC_FILESYSTEM_PROTOCOL_H__

#include <string>
#include <vector>
#include <map>
#include "CCPlatformMacros.h"
#include "ccTypes.h"
#include "ccTypeInfo.h"

NS_CC_BEGIN

enum FileSystemType
{	
	kAll,
    kResource,
	kAppData,
    kLua,
	kTemp
};

class CC_DLL CCFileSystemProtocol
{
public:
	static CCFileSystemProtocol* sharedFileSystem();

	virtual bool fileExists(FileSystemType type, const char* pszFileName) = 0;
	virtual unsigned char* fileRead(FileSystemType type, const char* pszFileName, unsigned long* pSize) = 0;
	virtual unsigned long fileWrite(FileSystemType type, const char* pszFileName, unsigned char* content, long size) = 0;
	virtual bool fileDelete(FileSystemType type, const char* pszFileName) = 0;

	virtual void reset() = 0;    

protected:
	static CCFileSystemProtocol* s_sharedFileSystem;    
};

typedef struct _FS4NItem {
	FileSystemType type;
	std::string path;
	bool readonly;
} FS4NItem;

class CC_DLL CCFileSystemBase : public CCFileSystemProtocol
{
public:   
	CCFileSystemBase();
    virtual ~CCFileSystemBase();

	void install();

	virtual bool fileExists(FileSystemType type, const char* pszFileName);
	virtual unsigned char* fileRead(FileSystemType type, const char* pszFileName, unsigned long* pSize);
	virtual unsigned long fileWrite(FileSystemType type, const char* pszFileName, unsigned char* content, long size);
	virtual bool fileDelete(FileSystemType type, const char* pszFileName);
    
	virtual void reset();    
	
	virtual std::string getPath(const FS4NItem* info, std::string name);

	void addSearchPath(FileSystemType type, const char* path, bool readonly);

protected:
	virtual bool fileExists(const char* pszFileName) = 0;
	virtual unsigned char* fileRead(const char* pszFileName, unsigned long* pSize) = 0;
	virtual unsigned long fileWrite(const char* pszFileName, unsigned char* content, long size) = 0;
	virtual bool fileDelete(const char* pszFileName) = 0;
    
protected:
    std::vector<FS4NItem> m_searchPaths;
};

NS_CC_END

#endif    // __CC_FILESYSTEM_PROTOCOL_H__
