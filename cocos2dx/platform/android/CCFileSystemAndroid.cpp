#include "CCFileSystemAndroid.h"
#include "support/zip_support/ZipUtils.h"
#include "platform/CCCommon.h"
#include "jni/Java_org_cocos2dx_lib_Cocos2dxHelper.h"

NS_CC_BEGIN

// record the zip on the resource path
static ZipFile *s_pZipFile = NULL;
CCFileSystemAndroid::CCFileSystemAndroid()
{
}

CCFileSystemAndroid::~CCFileSystemAndroid()
{
	if(s_pZipFile!=NULL)delete s_pZipFile;
}

ZipFile* getZipFile()
{
	if(s_pZipFile==NULL) {
		std::string resourcePath = getApkPath();
		s_pZipFile = new ZipFile(resourcePath, "assets/");
	}
	return s_pZipFile;
}

bool CCFileSystemAndroid::fileExists(const char* pszFileName)
{
	if (pszFileName[0] != '/')
    {
        std::string strPath = pszFileName;
        if (strPath.find("assets/") == 0)
        {
			return getZipFile()->fileExists(strPath);
        }
    }
	return access(pszFileName, 0)==0;
}

unsigned char* CCFileSystemAndroid::fileRead(const char* pszFileName, unsigned long* pSize)
{
	if (pszFileName[0] != '/')
    {
        std::string strPath = pszFileName;
        if (strPath.find("assets/") == 0)
        {
			return getZipFile()->getFileData(strPath.c_str(), pSize);
        }
	}
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

