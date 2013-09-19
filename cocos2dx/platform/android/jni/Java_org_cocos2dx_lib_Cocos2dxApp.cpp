#include "CCDirector.h"
#include "CCEConfig.h"
#include "../CCApplication.h"
#include "../CCFileSystemAndroid.h"
#include "CCEventType.h"
#include "support/CCNotificationCenter.h"
#include "JniHelper.h"
#include <jni.h>

using namespace cocos2d;

CCFileSystemAndroid g_fs;

extern "C" {

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxApp_nativeOnPause(JNIEnv* env, jobject thiz) {
        CCApplication::sharedApplication()->applicationDidEnterBackground();

        CCNotificationCenter::sharedNotificationCenter()->postNotification(EVENT_COME_TO_BACKGROUND, NULL);
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxApp_nativeOnResume(JNIEnv* env, jobject thiz) {
        if (CCDirector::sharedDirector()->getOpenGLView()) {
            CCApplication::sharedApplication()->applicationWillEnterForeground();
        }
    }
	
	JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxApp_nativeStartup(JNIEnv*  env, jobject thiz)
	{
		CCApplication::sharedApplication()->run();
	}
	
	JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxApp_nativeRun(JNIEnv* env, jobject thiz, jlong time) {
        CCApplication::sharedApplication()->run(time);
    }
	
	JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxApp_nativeConfig(JNIEnv* env, jobject thiz, jstring name, jint type, jint v1, jstring v2) {
		const char *strName = env->GetStringUTFChars(name, 0);
        switch(type) {
		case 0:
			CCEConfig::set(strName, CCValue::intValue(v1));			
			break;
		case 1: {
			const char *strValue = env->GetStringUTFChars(v2, 0);
			CCEConfig::set(strName, CCValue::stringValue(strValue));
			env->ReleaseStringUTFChars(v2, strValue);
			break;
			}
		case 2:
			CCEConfig::set(strName, CCValue::booleanValue(v1!=0));
			break;
		}
		env->ReleaseStringUTFChars(name, strName);
    }

	JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxApp_nativeAddFileSystem(JNIEnv* env, jobject thiz, jint type, jstring path, jboolean readonly) {
		if(CCFileSystem::sharedFileSystem()==NULL) {
			g_fs.install();
		}
		const char *strPath = env->GetStringUTFChars(path, 0);		
		g_fs.addSearchPath(FileSystemType(type), strPath, (bool) readonly);
		env->ReleaseStringUTFChars(path, strPath);
	}
}
