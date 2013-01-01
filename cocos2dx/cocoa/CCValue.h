#ifndef __CCVALUE_H__
#define __CCVALUE_H__

#include "platform/CCPlatformMacros.h"
#include <string>
#include <vector>
#include <map>

NS_CC_BEGIN

class CCValue;
class CCObject;
typedef std::map<std::string, CCValue>	CCValueMap;
typedef CCValueMap::const_iterator		CCValueMapIterator;
typedef std::vector<CCValue>			CCValueArray;
typedef CCValueArray::const_iterator	CCValueArrayIterator;

typedef enum {
	CCValueTypeNull,
    CCValueTypeInt,
    CCValueTypeNumber,
    CCValueTypeBoolean,
    CCValueTypeString,
    CCValueTypeMap,
    CCValueTypeArray,
	CCValueTypeObject,
	CCValueTypeFunction,
	CCValueTypeObjectCall,
} CCValueType;

typedef CCValue (*CC_FUNCTION_CALL)(CCValueArray&);
typedef CCValue (CCObject::*CC_OBJECT_CALL)(CCValueArray&);

#define ocall_selector(_SELECTOR) (CC_OBJECT_CALL)(&_SELECTOR)

typedef struct _CCOCall {
	CCObject* pObject;
	CC_OBJECT_CALL call;
} CCOCall;

typedef union {
    int					intValue;
    double				numberValue;
    bool				booleanValue;    
    CCValueMap*			mapValue;
    CCValueArray*		arrayValue;
	CCObject*			objectValue;
	CC_FUNCTION_CALL	fcallValue;
	CCOCall				ocallValue;
} CCValueField;

class CC_DLL CCValue
{
public:
	static std::string EMPTY;
	static CCValueArray EMPTY_PARAM;
	
public:
	static const CCValue nullValue();
    static const CCValue intValue(const int intValue);
    static const CCValue numberValue(const double numberValue);
    static const CCValue booleanValue(const bool booleanValue);
    static const CCValue stringValue(const char* stringValue);
    static const CCValue stringValue(const std::string& stringValue);
    static const CCValue mapValue(const CCValueMap& dictValue);
    static const CCValue arrayValue(const CCValueArray& arrayValue);    
	static const CCValue objectValue(CCObject* obj);
	static const CCValue fcallValue(CC_FUNCTION_CALL call);
	static const CCValue ocallValue(CCObject* obj, CC_OBJECT_CALL call);

    CCValue(void)
        : m_type(CCValueTypeNull)
    {
        memset(&m_field, 0, sizeof(m_field));
    }
    CCValue(const CCValue& rhs);
    CCValue& operator=(const CCValue& rhs);
    ~CCValue(void);

    const CCValueType getType(void) const {
        return m_type;
    }

	bool isNull() const {
		return m_type==CCValueTypeNull;
	}

	bool isInt() const {
		return m_type==CCValueTypeInt;
	}
    int intValue(void) const {
		if(m_type==CCValueTypeInt) {
			return m_field.intValue;
		}
		if(m_type==CCValueTypeNumber) {
			return (int) m_field.numberValue;
		}
		return 0;
    }

	bool isNumber() const {
		return m_type==CCValueTypeNumber;
	}
	float floatValue(void) const {
		return (float) numberValue();
	}
    double numberValue(void) const {
		if(m_type==CCValueTypeNumber) {
			return m_field.numberValue;
		}
		if(m_type==CCValueTypeInt) {
			return m_field.intValue;
		}
		return 0;
    }

	bool isBoolean() const {
		return m_type==CCValueTypeBoolean;
	}
    bool booleanValue(void) const {
		if(m_type==CCValueTypeBoolean) {
			return m_field.booleanValue;
		}
		if(m_type==CCValueTypeInt) {
			return m_field.intValue!=0;
		}
		if(m_type==CCValueTypeNumber) {
			return m_field.numberValue!=0;
		}		
		return false;
    }

	bool isString() const {
		return m_type==CCValueTypeString;
	}
    const std::string& stringValue(void) const {
		if(m_type==CCValueTypeString) {
			return m_fieldString;
		}
		return EMPTY;
    }

	bool isMap() const {
		return m_type==CCValueTypeMap;
	}
    const CCValueMap& mapValue(void) const {
		CC_ASSERT(isMap());
        return *m_field.mapValue;
    }

	bool isArray() const {
		return m_type==CCValueTypeArray;
	}
    const CCValueArray& arrayValue(void) const {
		CC_ASSERT(isArray());
        return *m_field.arrayValue;
    }

	bool isObject() const {
		return m_type==CCValueTypeObject;
	}
    CCObject* objectValue(void) const {
		if(isObject()) {
			return m_field.objectValue;
		}
		return NULL;
	}

	bool canCall() const;
	bool isFunction() const {
		return m_type==CCValueTypeFunction;
	}
    const CC_FUNCTION_CALL fcallValue(void) const {
		if(isFunction()) {
			return m_field.fcallValue;
		}
		return NULL;
    }

	bool isObjectCall() const {
		return m_type==CCValueTypeObjectCall;
	}
    const CCOCall* ocallValue(void) const {
		if(isObjectCall()) {
			return &m_field.ocallValue;
		}
		return NULL;
    }

	void retain();
	void cleanup();

	CCValue call(CCValueArray& params,bool throwErr);
	bool callback(std::string err, CCValue result);
	bool result(CCValue result){return callback(EMPTY, result);};
	bool error(std::string err){return callback(err, nullValue());};

private:
    CCValueField m_field;
	bool m_retain;
	std::string	 m_fieldString;
    CCValueType  m_type;

    void copy(const CCValue& rhs);
};

NS_CC_END

#endif // __CCVALUE_H__
