#ifndef  __CCE_CONFIG_H__
#define  __CCE_CONFIG_H__

#include "platform/CCPlatformMacros.h"
#include "cocoa/CCObject.h"
#include "ccTypes.h"
#include "ccTypeInfo.h"

NS_CC_BEGIN

class CC_DLL CCEConfig
{
public:
	CCEConfig();
	virtual ~CCEConfig(void);

public:
	static CCValue get(const char* name);
	static void set(const char* name, CCValue v);
	
	static bool isDebug(){return get("debug").booleanValue();};

protected:
	CCValueMap m_data;
};

NS_CC_END

#endif  // __CC_CONFIG_H__