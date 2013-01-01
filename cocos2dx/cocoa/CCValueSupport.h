#ifndef __CCVALUE_COMMON_H__
#define __CCVALUE_COMMON_H__

#include "CCValue.h"

NS_CC_BEGIN

#define CALLNAME(name) call_##name

#define CC_BEGIN_CALLS(CLASS,SCLASS) static CALL_INFO s_CALLS_##CLASS[] = {
#define CC_DEFINE_CALL(CLASS, name) {#name, (CC_OBJECT_CALL) &CLASS::call_##name},
#define CC_END_CALLS(CLASS,SCALL) {NULL,NULL}}; \
	bool CLASS::canCall(const char* method){ \
		if(CCObject::canCallImpl(this, s_CALLS_##CLASS, method)) { return true; } \
		return SCALL::canCall(method); \
	} \
	CCValue CLASS::call(const char* method, CCValueArray& params){ \
		if(method==NULL)return invoke(params); \
		CCValue r; \
		if(CCObject::callImpl(this, s_CALLS_##CLASS, r, method, params)) { return r; } \
		return SCALL::call(method, params); \
	}

NS_CC_END

#endif // __CCVALUE_H__
