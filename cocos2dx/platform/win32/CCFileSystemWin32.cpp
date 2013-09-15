#include "CCFileSystemWin32.h"
#include <list>

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

bool direxists(const char* folderName) {
	 DWORD attr = GetFileAttributesA(folderName);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        //File not found
        return false;
    }   
    if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        // File is not a directory
        return false;
    }
    return true;
}

bool mkdirs(const char* strFolder) {
    std::list<std::string> folderLevels;

	std::string folderName(strFolder);
    char* c_str = (char*)folderName.c_str();

    // Point to end of the string
    char* strPtr = &c_str[strlen(c_str) - 1];

    // Create a list of the folders which do not currently exist
    do {
		if (direxists(c_str)) {
            break;
        }
        // Break off the last folder name, store in folderLevels list
        do {
            strPtr--;
        } while ((*strPtr != '\\') && (*strPtr != '/') && (strPtr >= c_str));
        folderLevels.push_front(std::string(strPtr + 1));
        strPtr[1] = 0;
    } while (strPtr >= c_str);

	std::string path(c_str);

    // Create the folders iteratively
    for (std::list<std::string>::iterator it = folderLevels.begin(); it != folderLevels.end(); it++) {
		path += it->c_str();
        if (!CreateDirectoryA(path.c_str(), NULL)) {
            return false;
        }
    }
    return true;
}

unsigned long CCFileSystemWin32::fileWrite(const char* pszFileName, unsigned char* content, long size)
{
	std::string fpath;
	char full[_MAX_PATH];
	if( _fullpath( full, pszFileName, _MAX_PATH ) != NULL ) {
		fpath = full;
		char* lpfile;
		if(GetFullPathNameA(fpath.c_str(), _MAX_PATH, full, &lpfile)!=0) {
			if(lpfile) {
				fpath = std::string(full, lpfile-full-1);
			} else {
				fpath = full;
			}
		} else {
			fpath = "";
		}
	}

	if(fpath.size()>0) {
		mkdirs(fpath.c_str());
	}

	FILE *fp = fopen(pszFileName, "wb");
	bool r = false;
	size_t wcount = 0;
	if(fp) {
		wcount = fwrite(content, 1,size, fp);
		fclose(fp);	
	}
	return wcount;
}

bool CCFileSystemWin32::fileDelete(const char* pszFileName)
{
	return ::DeleteFileA(pszFileName)==TRUE;
}

NS_CC_END

