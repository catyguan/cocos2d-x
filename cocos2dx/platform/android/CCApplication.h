#ifndef __CC_APPLICATION_ANDROID_H__
#define __CC_APPLICATION_ANDROID_H__

#include "platform/CCCommon.h"
#include "platform/CCApplicationProtocol.h"

#define MAX_APP_RUNNABLE	5

NS_CC_BEGIN

class CCRect;

class CC_DLL CCApplication : public CCApplicationProtocol
{
public:
    CCApplication();
    virtual ~CCApplication();

    /**
    @brief    Callback by CCDirector to limit FPS.
    @interval       The time, expressed in seconds, between current frame and next. 
    */
    void setAnimationInterval(double interval);

    /**
    @brief    Run the message loop.
    */
    int run();
	void run(long time);

    /**
    @brief    Get current application instance.
    @return Current application instance pointer.
    */
    static CCApplication* sharedApplication();

    /**
    @brief Get current language config
    @return Current language config
    */
    virtual ccLanguageType getCurrentLanguage();
    
    /**
     @brief Get target platform
     */
    virtual TargetPlatform getTargetPlatform();
	
	// catyguan TODO
	virtual void addRunnable(int pos, CCApplicationRunnable runnable, void* data);
	virtual void removeRunnable(int pos);
	virtual void resetApplication(){m_reset = true;};

protected:	
	virtual void startApplication() = 0;
	virtual void resetCloseApplication() = 0;

protected:
	CCApplicationRunnable appRun[MAX_APP_RUNNABLE];
	void* appRunData[MAX_APP_RUNNABLE];
	bool m_reset;
	
    static CCApplication * sm_pSharedApplication;
};

NS_CC_END

#endif    // __CC_APPLICATION_ANDROID_H__
