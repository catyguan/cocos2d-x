#ifndef __CC_APPLICATION_WIN32_H__
#define __CC_APPLICATION_WIN32_H__

#include "CCStdC.h"
#include "platform/CCCommon.h"
#include "platform/CCApplicationProtocol.h"
#include <string>

#define MAX_APP_RUNNABLE	5

NS_CC_BEGIN

class CCRect;

class CC_DLL CCApplication : public CCApplicationProtocol
{
public:
    CCApplication();
    virtual ~CCApplication();

    /**
    @brief    Run the message loop.
    */
    int run();

    /**
    @brief    Get current applicaiton instance.
    @return Current application instance pointer.
    */
    static CCApplication* sharedApplication();

    /* override functions */
    virtual void setAnimationInterval(double interval);
    virtual ccLanguageType getCurrentLanguage();
    
    /**
     @brief Get target platform
     */
    virtual TargetPlatform getTargetPlatform();

    /**
     *  Sets the Resource root path.
     *  @deprecated Please use CCFileUtils::sharedFileUtils()->setSearchPaths() instead.
     */
    CC_DEPRECATED_ATTRIBUTE void setResourceRootPath(const std::string& rootResDir);

    /** 
     *  Gets the Resource root path.
     *  @deprecated Please use CCFileUtils::sharedFileUtils()->getSearchPaths() instead. 
     */
    CC_DEPRECATED_ATTRIBUTE const std::string& getResourceRootPath(void);

    void setStartupScriptFilename(const std::string& startupScriptFile);

    const std::string& getStartupScriptFilename(void)
    {
        return m_startupScriptFilename;
    }

	virtual void addRunnable(int pos, CCApplicationRunnable runnable, void* data);
	virtual void removeRunnable(int pos);
	virtual void resetApplication(){m_reset = true;};

protected:	
	virtual void startApplication() = 0;
	virtual void resetCloseApplication() = 0;

protected:
    HINSTANCE           m_hInstance;
    HACCEL              m_hAccelTable;
	LARGE_INTEGER		m_nFreq;
	LARGE_INTEGER		m_nStart;
    LARGE_INTEGER       m_nAnimationInterval;
    std::string         m_resourceRootPath;
    std::string         m_startupScriptFilename;

	CCApplicationRunnable appRun[MAX_APP_RUNNABLE];
	void* appRunData[MAX_APP_RUNNABLE];
	bool m_reset;

    static CCApplication * sm_pSharedApplication;
};

NS_CC_END

#endif    // __CC_APPLICATION_WIN32_H__
