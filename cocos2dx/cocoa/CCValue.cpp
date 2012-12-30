#include "CCValue.h"
#include "CCObject.h"
#include "platform/CCCommon.h"

NS_CC_BEGIN

// CCValue
std::string CCValue::EMPTY;
CCValueArray CCValue::EMPTY_PARAM;

std::string REF_ONCE("11");
std::string NOREF_NOONCE("00");
std::string NOREF_ONCE("01");
std::string REF_NOONCE("10");

#define ROSTRING(r,o) r?(o?REF_ONCE:REF_NOONCE):(o?NOREF_ONCE:NOREF_NOONCE)

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
    return value;
}

const CCValue CCValue::numberValue(const double numberValue)
{
    CCValue value;
    value.m_type = CCValueTypeNumber;
    value.m_field.numberValue = numberValue;
    return value;
}

const CCValue CCValue::booleanValue(const bool booleanValue)
{
    CCValue value;
    value.m_type = CCValueTypeBoolean;
    value.m_field.booleanValue = booleanValue;
    return value;
}

const CCValue CCValue::stringValue(const char* stringValue)
{
    CCValue value;
    value.m_type = CCValueTypeString;
    value.m_fieldString = stringValue;
    return value;
}

const CCValue CCValue::stringValue(const std::string& stringValue)
{
    CCValue value;
    value.m_type = CCValueTypeString;
    value.m_fieldString = stringValue;
    return value;
}

const CCValue CCValue::mapValue(const CCValueMap& dictValue)
{
    CCValue value;
    value.m_type = CCValueTypeMap;
    value.m_field.mapValue = new CCValueMap(dictValue);
    return value;
}

const CCValue CCValue::arrayValue(const CCValueArray& arrayValue)
{
    CCValue value;
    value.m_type = CCValueTypeArray;
    value.m_field.arrayValue = new CCValueArray(arrayValue);
    return value;
}

const CCValue CCValue::objectValue(CCObject* obj,bool ref)
{
	if(obj==NULL)return nullValue();

	CCValue value;
	value.m_type = CCValueTypeObject;
	value.m_field.objectValue = obj;
	if(ref) {
		obj->retain();
	} else {
		value.m_fieldString = ROSTRING(ref,false);
	}
	return value;
}

const CCValue CCValue::fcallValue(CC_FUNCTION_CALL call)
{
	if(call==NULL)return nullValue();

	CCValue value;
	value.m_type = CCValueTypeFunction;
	value.m_field.fcallValue = call;
	return value;
}

const CCValue CCValue::ocallValue(CCObject* obj, CC_OBJECT_CALL call,bool ref,bool once)
{	
	if(obj==NULL || call==NULL)return nullValue();	

	CCValue value;
	value.m_type = CCValueTypeObjectCall;
	value.m_field.ocallValue.pObject = obj;
	value.m_field.ocallValue.call = call;
	if(ref) {
		obj->retain();
	} else {
		value.m_fieldString = ROSTRING(ref,once);
	}
	return value;
}

const CCValue CCValue::oacallValue(CCObject* obj, CC_OBJECT_ACALL call,bool ref,bool once)
{
	if(obj==NULL || call==NULL)return nullValue();	

	CCValue value;
	value.m_type = CCValueTypeObjectCall;
	value.m_field.oacallValue.pObject = obj;
	value.m_field.oacallValue.call = call;
	if(ref) {
		obj->retain();
	} else {
		value.m_fieldString = ROSTRING(ref,once);
	}
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
	else if(m_type == CCValueTypeObject || m_type==CCValueTypeObjectCall || m_type==CCValueTypeObjectACall)
	{
		if(m_fieldString[0]=='1') {
			CC_SAFE_RELEASE_NULL(m_field.objectValue);
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
    memcpy(&m_field, &rhs.m_field, sizeof(m_field));
    m_type = rhs.m_type;
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
	else if(m_type == CCValueTypeObject || m_type==CCValueTypeObjectCall || m_type==CCValueTypeObjectACall)
	{
		m_fieldString = rhs.m_fieldString;
		if(m_fieldString[0]=='1') {
			if(m_type==CCValueTypeObject) {
				m_field.objectValue->retain();
			}
			else if(m_type==CCValueTypeObjectCall) {
				m_field.ocallValue.pObject->retain();
			}
			else if(m_type==CCValueTypeObjectACall) {
				m_field.oacallValue.pObject->retain();
			}
		}
	}
}

CCValue CCValue::ref()
{
	if(m_type == CCValueTypeObject || m_type==CCValueTypeObjectCall || m_type==CCValueTypeObjectACall)
	{
		if(m_fieldString[0]=='0') {
			if(m_type==CCValueTypeObject) {
				return CCValue::objectValue(m_field.objectValue, true);
			}
			else if(m_type==CCValueTypeObjectCall) {
				return CCValue::ocallValue(m_field.ocallValue.pObject,m_field.ocallValue.call,true,isCallOnce());
			}
			else if(m_type==CCValueTypeObjectACall) {
				return CCValue::oacallValue(m_field.oacallValue.pObject,m_field.oacallValue.call,true,isCallOnce());
			}
		}
	}
	return CCValue(*this);
}

bool CCValue::isCallOnce()
{
	return m_fieldString.size()>1 && m_fieldString[1]=='1';
}

CCValue CCValue::call(CCValueArray& params,bool throwErr)
{
	try {
		if(m_type==CCValueTypeFunction) {
			CCValue r = m_field.fcallValue(params);
			if(isCallOnce())cleanup();
			return r;
		} else if(m_type==CCValueTypeObjectCall){
			CCValue r = (m_field.ocallValue.pObject->*m_field.ocallValue.call)(params);
			if(isCallOnce())cleanup();
			return r;
		} else if(m_type==CCValueTypeObjectCall) {
			bool r = (m_field.oacallValue.pObject->*m_field.oacallValue.call)(CCValue::nullValue(), params);
			if(isCallOnce())cleanup();
			return CCValue::booleanValue(r);
		} else {
			throw std::string("invalid value type[%d] for call",m_type);
		}
	} catch(std::string err) {		
		if(throwErr) {
			throw err;
		} else {
			CCLOG("skip call err - %s", err.c_str());
			return nullValue();
		}
	}
}

bool CCValue::acall(CCValue callback, CCValueArray& params)
{
	try {
		if(m_type==CCValueTypeFunction) {
			CCValue r = m_field.fcallValue(params);
			if(isCallOnce())cleanup();
			return callback.callback(EMPTY, r);
		} else if(m_type==CCValueTypeObjectCall){
			CCValue r = (m_field.ocallValue.pObject->*m_field.ocallValue.call)(params);
			if(isCallOnce())cleanup();
			return callback.callback(EMPTY, r);
		} else if(m_type==CCValueTypeObjectCall) {
			bool r = (m_field.oacallValue.pObject->*m_field.oacallValue.call)(CCValue::nullValue(), params);
			if(isCallOnce())cleanup();
			return r;
		} else {
			throw std::string("invalid value type[%d] for acall",m_type);
		}
	} catch(std::string err) {		
		callback.callback(err, CCValue::nullValue());
		return true;
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
