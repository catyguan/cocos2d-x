#include "text_input_node/CCIMEDispatcher.h"
#include "CCDirector.h"
#include "../CCApplication.h"
#include "platform/CCFileUtils.h"
#include "CCEventType.h"
#include "support/CCNotificationCenter.h"
#include "JniHelper.h"
#include <jni.h>

using namespace cocos2d;

extern "C" {

	JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeInit(JNIEnv*  env, jobject thiz, jint w, jint h)
	{
		if (!CCDirector::sharedDirector()->getOpenGLView())
		{
			CCEGLView *view = CCEGLView::sharedOpenGLView();
			view->setFrameSize(w, h);
		}
	}
	
    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeRender(JNIEnv* env, jobject thiz) {
        cocos2d::CCDirector::sharedDirector()->mainLoop();
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeInsertText(JNIEnv* env, jobject thiz, jstring text) {
        const char* pszText = env->GetStringUTFChars(text, NULL);
        cocos2d::CCIMEDispatcher::sharedDispatcher()->dispatchInsertText(pszText, strlen(pszText));
        env->ReleaseStringUTFChars(text, pszText);
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeDeleteBackward(JNIEnv* env, jobject thiz) {
        cocos2d::CCIMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
    }

    JNIEXPORT jstring JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeGetContentText(JNIEnv* env, jobject thiz) {
        JNIEnv * env = 0;

        if (JniHelper::getJavaVM()->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK || ! env) {
            return 0;
        }
        const char * pszText = cocos2d::CCIMEDispatcher::sharedDispatcher()->getContentText();
        return env->NewStringUTF(pszText);
    }
	
	
}
