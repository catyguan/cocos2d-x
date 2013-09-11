#include "CCFileSystemAndroid.h"

NS_CC_BEGIN

CCFileSystemAndroid::CCFileSystemAndroid()
{
}

CCFileSystemAndroid::~CCFileSystemAndroid()
{
}

bool CCFileSystemAndroid::fileExists(const char* pszFileName)
{
	 return access(pszFileName, 0)==0;
}

unsigned char* CCFileSystemAndroid::fileRead(const char* pszFileName, unsigned long* pSize)
{
	unsigned char * pData = 0;
	unsigned long size = 0;	
	do 
    {
        FILE *fp = fopen(pszFileName, "rb");
        CC_BREAK_IF(!fp);
        
        fseek(fp,0,SEEK_END);
        size = ftell(fp);
        fseek(fp,0,SEEK_SET);
        pData = new unsigned char[size];
        size = fread(pData,sizeof(unsigned char), size,fp);
        fclose(fp);		
	} while (0);    
	*pSize = size;
    return pData;
}

unsigned long CCFileSystemAndroid::fileWrite(const char* pszFileName, unsigned char* content, long size)
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

bool CCFileSystemAndroid::fileDelete(const char* pszFileName)
{
	return remove(pszFileName)==0;
}

NS_CC_END

