#ifndef __CCVALUE_COMMON_H__
#define __CCVALUE_COMMON_H__

#include "ccTypes.h"
#include "CCValue.h"
#include "CCGeometry.h"

NS_CC_BEGIN

class CC_DLL CCCommandObject : public CCObject
{
public:
    //super methods
	virtual CCValue invoke(CCValueArray& params);
	virtual void cleanup();

public:
    /** Allocates and initializes the action */
	static CCCommandObject* create(CCObject* obj, const char* method, CCValueArray& params);
	static CCCommandObject* create(CCObject* obj, const char* method);

protected:
	void initObjectCommand(CCObject* obj,const char* method,CCValueArray& ps);

	CCCommandObject();
	virtual ~CCCommandObject();

protected:
	CCObject* m_pObject;
	std::string m_csName;
	CCValueArray m_Params;
};

class CC_DLL CCValueUtil
{
public:
	static void append(CCValueArray& r,CCValueArray& a1);

	static CCPoint point(CCValue& v);
	static CCSize size(CCValue& v);
	static CCRect rect(CCValue& v);
	static ccColor4B color4b(CCValue& v);
	static ccColor3B color3b(CCValue& v);

	static CCValue size(float x, float y);
	static CCValue point(float x, float y);	
	static CCValue color3b(ccColor3B& v);
};

#define ccvp(params,idx) (params.size()>idx?params[idx]:CCValue::nullValue())
#define ccvpObject(params,idx,otype)	((otype*)(params.size()>idx?(params[idx].isObject()?dynamic_cast<otype*>(params[idx].objectValue()):NULL):NULL))
#define ccvpMap(params,idx)	(params.size()>idx?params[idx].mapValue():NULL)
#define ccvpBoolean(params,idx)	(params.size()>idx?params[idx].booleanValue():false)
#define ccvpInt(params,idx)	(params.size()>idx?params[idx].intValue():0)
#define ccvpFloat(params,idx)	(params.size()>idx?params[idx].floatValue():0)
#define ccvpString(params,idx)	(params.size()>idx?params[idx].stringValue():std::string())
#define ccvpSize(params, idx) (params.size()>idx?CCValueUtil::size(params[idx]):CCSizeMake(0,0))
#define ccvpRect(params, idx) (params.size()>idx?CCValueUtil::rect(params[idx]):CCRectMake(0,0,0,0))
#define ccvpPoint(params, idx) (params.size()>idx?CCValueUtil::point(params[idx]):CCPointZero)
#define ccvpColor(params, idx) (params.size()>idx?CCValueUtil::color4b(params[idx]):ccc4(0,0,0,255))
#define ccvpColor3B(params, idx) (params.size()>idx?CCValueUtil::color3b(params[idx]):ccc3(0,0,0))

#define CALLNAME(name) call_##name

#define CC_BEGIN_CALLS(CLASS,SCLASS) static CALL_INFO s_CALLS_##CLASS[] = {
#define CC_DEFINE_CALL(CLASS, name) {#name, (CC_OBJECT_CALL) &CLASS::call_##name},
#define CC_DEFINE_CALL_ALIAS(CLASS, name, newname) {#name, (CC_OBJECT_CALL) &CLASS::call_##newname},
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
