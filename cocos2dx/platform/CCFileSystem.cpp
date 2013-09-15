#include "CCFileSystem.h"
#include "CCCommon.h"

NS_CC_BEGIN

CCFileSystem* CCFileSystem::s_sharedFileSystem; 

CCFileSystem* CCFileSystem::sharedFileSystem()
{
	return s_sharedFileSystem;
}

std::string CCFileSystem::fileReadString(FileSystemType type, const char* pszFileName)
{
	unsigned long size;
	unsigned char* buf = fileRead(type, pszFileName, &size);
	if(buf!=NULL) {
		std::string r((char*) buf, size);
		delete[] buf;
		return r;
	}
	return "";
}

bool CCFileSystem::fileWriteString(FileSystemType type, const char* pszFileName, std::string content)
{
	size_t size = content.size();
	return fileWrite(type,pszFileName, (unsigned char*) content.c_str(), size)==size;
}

CCFileSystemBase::CCFileSystemBase()
{

}

CCFileSystemBase::~CCFileSystemBase()
{

}

void CCFileSystemBase::install()
{
	s_sharedFileSystem = this;
}
    
void CCFileSystemBase::reset()
{

}

std::string CCFileSystemBase::getPath(const FS4NItem* info, std::string name)
{
	if(name.length()>1 && name[0]=='/') {
		name = name.substr(1);
	}
	std::string path = info->path;
	return path+name;
}
    
bool CCFileSystemBase::fileExists(FileSystemType type, const char* vpathName)
{
	std::string name(vpathName);
	std::vector<FS4NItem>::const_iterator it = m_searchPaths.begin();
	for(;it!=m_searchPaths.end();it++) {
		if(it->type==kAll || it->type==type) {
			std::string file = getPath(&(*it), name);
			if(fileExists(file.c_str())) {
				return true;
			}			
		}
	}
	return false;
}

unsigned char* CCFileSystemBase::fileRead(FileSystemType type, const char* vpathName, unsigned long* pSize)
{
	std::string name(vpathName);
	std::vector<FS4NItem>::const_iterator it = m_searchPaths.begin();
	for(;it!=m_searchPaths.end();it++) {
		if(it->type==kAll || it->type==type) {
			std::string file = getPath(&(*it), name);
			if(fileExists(file.c_str())) {
				return fileRead(file.c_str(), pSize);
			}			
		}
	}
	*pSize = 0;
	return NULL;
}

unsigned long CCFileSystemBase::fileWrite(FileSystemType type, const char* pszFileName, unsigned char* content, long size)
{
	std::string name(pszFileName);
	std::vector<FS4NItem>::const_iterator it = m_searchPaths.begin();
	for(;it!=m_searchPaths.end();it++) {
		if(it->readonly)continue;
		if(it->type==kAll || it->type==type) {
			std::string file = getPath(&(*it), name);
			return fileWrite(file.c_str(), content, size);						
		}
	}
	return 0;
}

bool CCFileSystemBase::fileDelete(FileSystemType type, const char* pszFileName)
{
	bool r = false;
	std::string name(pszFileName);
	std::vector<FS4NItem>::const_iterator it = m_searchPaths.begin();
	for(;it!=m_searchPaths.end();it++) {
		if(it->readonly)continue;
		if(it->type==kAll || it->type==type) {
			std::string file = getPath(&(*it), name);
			if(fileDelete(file.c_str()))r = true;
		}
	}
	return r;
}

void CCFileSystemBase::addSearchPath(FileSystemType type, const char* path, bool readonly)
{
	FS4NItem item;
	item.type = type;
	item.path = path;
	item.readonly = readonly;
	m_searchPaths.push_back(item);
}   

NS_CC_END

