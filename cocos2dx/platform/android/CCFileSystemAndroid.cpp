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
	// CCLOG("check %s", pszFileName);
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

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

static int make_dirs(char *new_path, int perms)
{
    char *saved_path, *cp;
    int saved_ch;
    struct stat st;
    int rc;
   
    cp = saved_path = strdup(new_path);
    while (*cp && *cp == '/') ++cp;
  
    while (1) {
       while (*cp && *cp != '/') ++cp;
       if ((saved_ch = *cp) != 0)
           *cp = 0;
           
       if ((rc = stat(saved_path, &st)) >= 0) {
           if (!S_ISDIR(st.st_mode)) {
		   CCLOG("%s not dir", saved_path);
               errno = ENOTDIR;
               rc = -1;
               break;
           }
       } 
       else {
           if (errno != ENOENT ) {
			CCLOG("%s stat fail 1", saved_path);
              break;
           }
           
		   CCLOG("mkdir %s", saved_path);
           if ((rc = mkdir(saved_path, perms)) < 0 ) {
				CCLOG("mkdir %s fail %d,%d", saved_path,rc,errno);
               if (errno != EEXIST)
                   break;
               
               if ((rc = stat(saved_path, &st)) < 0)
                   break;
                   
               if (!S_ISDIR(st.st_mode)) {
                   errno = ENOTDIR;
                   rc = -1;
                   break;
               }                
           }
       }
       
       if (saved_ch != 0)
           *cp = saved_ch;
       
       while (*cp && *cp == '/') ++cp;
       if (*cp == 0)
           break;
   }
   
   free(saved_path);
   return rc;
}

unsigned long CCFileSystemAndroid::fileWrite(const char* pszFileName, unsigned char* content, long size)
{
	char dirname[1024];
	int pos = -1;
	int c = strlen(pszFileName);
	for(int i=c-1;i>0;i--) {
		if(pszFileName[i]=='/') {
			pos = i;
			break;
		}
	}
	if(pos>0) {
		strncpy(dirname,pszFileName,pos);
		dirname[pos] = 0;
		make_dirs(dirname, 0777);
    }
	FILE *fp = fopen(pszFileName, "wb");
	bool r = false;
	size_t wcount = 0;
	if(fp) {
		wcount = fwrite(content, 1, size, fp);
		fclose(fp);	
	} else {
		CCLOG("fileWrite(%s) open fail", pszFileName);
	}
	return wcount;
}

bool CCFileSystemAndroid::fileDelete(const char* pszFileName)
{
	return remove(pszFileName)==0;
}

NS_CC_END

