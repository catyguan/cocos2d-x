#ifndef __CCX_APPLICATION_WIN32_H__
#define __CCX_APPLICATION_WIN32_H__

#include <Windows.h>

#include "ccxCommon.h"

NS_CC_BEGIN;

class CGRect;

class CCX_DLL ccxApplication
{
public:
    ccxApplication();
    virtual ~ccxApplication();

    /**
    @brief	Implement for initialize OpenGL instance, set source path, etc...
    */
    virtual bool initInstance() = 0;

    /**
    @brief	Implement CCDirector and CCScene init code here.
    @return true    Initialize success, app continue.
    @return false   Initialize failed, app terminate.
    */
    virtual bool applicationDidFinishLaunching() = 0;

    /**
    @brief  The function be called when the application enter background
    @param  the pointer of the application
    */
    virtual void applicationDidEnterBackground() = 0;

    /**
    @brief  The function be called when the application enter foreground
    @param  the pointer of the application
    */
    virtual void applicationWillEnterForeground() = 0;

    /**
    @brief	Callback by CCDirector for limit FPS.
    @interval       The time, which expressed in second in second, between current frame and next. 
    */
    void setAnimationInterval(double interval);

    typedef enum
    {
        /// Device oriented vertically, home button on the bottom
        kOrientationPortrait = 0,
        /// Device oriented vertically, home button on the top
        kOrientationPortraitUpsideDown = 1,
        /// Device oriented horizontally, home button on the right
        kOrientationLandscapeLeft = 2,
        /// Device oriented horizontally, home button on the left
        kOrientationLandscapeRight = 3,
    } Orientation;

    /**
    @brief	Callback by CCDirector for change device orientation.
    @orientation    The defination of orientation which CCDirector want change to.
    @return         The actual orientation of the application.
    */
    Orientation setOrientation(Orientation orientation);

    /**
    @brief	Get status bar rectangle in EGLView window.
    */
    void    statusBarFrame(CGRect * rect);

    /**
    @brief	Run the message loop.
    */
    int run();

    /**
    @brief	Get current applicaiton instance.
    @return Current application instance pointer.
    */
    static ccxApplication& sharedApplication();

protected:
    HINSTANCE           m_hInstance;
    HACCEL              m_hAccelTable;
    LARGE_INTEGER       m_nAnimationInterval;

    static ccxApplication * sm_pSharedApplication;
};

NS_CC_END;

#endif	// __CCX_APPLICATION_WIN32_H__