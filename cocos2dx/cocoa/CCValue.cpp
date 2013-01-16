#include "CCValue.h"
#include "CCObject.h"
#include "platform/CCCommon.h"

NS_CC_BEGIN

// CCValue
std::string CCValue::EMPTY;
CCValueArray CCValue::EMPTY_PARAM;

const CCValue CCValue::nullValue()
{
	CCValue value;
    value.m_type = CCValueTypeNull;
    value.m_field.intValue = 0;
    return value;
}

const CCValue CCValue::intValue(const int intValue)
{
    CCValue value;
    value.m_type = CCValueTypeInt;
    value.m_field.intValue = intValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::numberValue(const double numberValue)
{
    CCValue value;
    value.m_type = CCValueTypeNumber;
    value.m_field.numberValue = numberValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::booleanValue(const bool booleanValue)
{
    CCValue value;
    value.m_type = CCValueTypeBoolean;
    value.m_field.booleanValue = booleanValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::stringValue(const char* stringValue)
{
    CCValue value;
    value.m_type = CCValueTypeString;
    value.m_fieldString = stringValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::stringValue(const std::string& stringValue)
{
    CCValue value;
    value.m_type = CCValueTypeString;
    value.m_fieldString = stringValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::mapValue(const CCValueMap& dictValue)
{
    CCValue value;
    value.m_type = CCValueTypeMap;
    value.m_field.mapValue = new CCValueMap(dictValue);
	value.m_retain = false;
    return value;
}

const CCValue CCValue::arrayValue(const CCValueArray& arrayValue)
{
    CCValue value;
    value.m_type = CCValueTypeArray;
    value.m_field.arrayValue = new CCValueArray(arrayValue);
	value.m_retain = false;
    return value;
}

const CCValue CCValue::objectValue(CCObject* obj)
{
	if(obj==NULL)return nullValue();

	CCValue value;
	value.m_type = CCValueTypeObject;
	value.m_field.objectValue = obj;
	value.m_retain = false;
	return value;
}

const CCValue CCValue::fcallValue(CC_FUNCTION_CALL call)
{
	if(call==NULL)return nullValue();

	CCValue value;
	value.m_type = CCValueTypeFunction;
	value.m_field.fcallValue = call;
	value.m_retain = false;
	return value;
}

const CCValue CCValue::ocallValue(CCObject* obj, CC_OBJECT_CALL call)
{	
	if(obj==NULL || call==NULL)return nullValue();	

	CCValue value;
	value.m_type = CCValueTypeObjectCall;
	value.m_field.ocallValue.pObject = obj;
	value.m_field.ocallValue.call = call;
	value.m_retain = false;
	return value;
}

CCValue::CCValue(const CCValue& rhs)
{
    copy(rhs);
}

CCValue& CCValue::operator=(const CCValue& rhs)
{
    if (this != &rhs) copy(rhs);
    return *this;
}

void CCValue::cleanup()
{
	if (m_type == CCValueTypeMap)
    {
		if(m_field.mapValue!=NULL) {
			delete m_field.mapValue;
		}		
		m_field.mapValue = NULL;
    }
    else if (m_type == CCValueTypeArray)
    {
		if(m_field.arrayValue!=NULL) {
			delete m_field.arrayValue;
		}
		m_field.arrayValue = NULL;
    }
	else if(m_type == CCValueTypeObject || m_type==CCValueTypeObjectCall)
	{
		if(m_retain) {
			if(m_type==CCValueTypeObject) {
				// CCLOG("--- release 1");
				CC_SAFE_RELEASE_NULL(m_field.objectValue);
			}
			else if(m_type==CCValueTypeObjectCall) {
				// CCLOG("--- oc release 1");
				CC_SAFE_RELEASE_NULL(m_field.ocallValue.pObject);
			}
		}
	}
	m_type = CCValueTypeNull;
}

CCValue::~CCValue(void)
{
	cleanup();    
}

void CCValue::copy(const CCValue& rhs)
{
	cleanup();
    memcpy(&m_field, &rhs.m_field, sizeof(m_field));
    m_type = rhs.m_type;
	m_retain = false;
    if (m_type == CCValueTypeString)
    {
        m_fieldString = rhs.m_fieldString;
    }
    else if (m_type == CCValueTypeMap)
    {
        m_field.mapValue = new CCValueMap(*rhs.m_field.mapValue);
    }
    else if (m_type == CCValueTypeArray)
    {
        m_field.arrayValue = new CCValueArray(*rhs.m_field.arrayValue);
    }	
}

void CCValue::retain()
{
	if(m_type == CCValueTypeObject || m_type==CCValueTypeObjectCall)
	{
		if(!m_retain) {
			m_retain = true;
			if(m_type==CCValueTypeObject) {
				// CCLOG("retain 1");
				CC_SAFE_RETAIN(m_field.objectValue);
			}
			else if(m_type==CCValueTypeObjectCall) {
				// CCLOG("retain 1");
				CC_SAFE_RETAIN(m_field.ocallValue.pObject);
			}
		}
	}
}

bool CCValue::canCall() const {
	return m_type==CCValueTypeFunction || m_type==CCValueTypeObjectCall || m_type==CCValueTypeObject;
}

CCValue CCValue::call(CCValueArray& params,bool throwErr)
{
	try {
		if(m_type==CCValueTypeFunction) {
			return m_field.fcallValue(params);
		} else if(m_type==CCValueTypeObjectCall){
			return (m_field.ocallValue.pObject->*m_field.ocallValue.call)(params);
		} else if(m_type==CCValueTypeObject){
			return m_field.objectValue->invoke(params);
		} else {
			char buf[128];
			sprintf(buf,"invalid value type[%d] for call",m_type);
			throw new std::string(buf);
		}
	} catch(std::string* err) {		
		if(throwErr) {
			throw err;
		} else {
			CCLOG("skip call err - %s", err->c_str());
			delete err;
			return nullValue();
		}
	}
}

bool CCValue::callback(std::string err, CCValue result)
{
	if(isNull())return false;
	CCValueArray ps;
	ps.push_back(stringValue(err));
	ps.push_back(result);
	CCValue r = call(ps, false);
	return r.booleanValue();
}

NS_CC_END
