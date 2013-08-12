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
    const std::string stringValue(void) const {
		if(m_type==CCValueTypeString) {
			return m_fieldString;
		}
		return "";
    }

	bool isMap() const {
		return m_type==CCValueTypeMap;
	}
    CCValueMap* mapValue(void) const {
		if(isMap()) {
			return m_field.mapValue;
		}
		return NULL;
    }

	bool isArray() const {
		return m_type==CCValueTypeArray;
	}
    CCValueArray* arrayValue(void) const {
		if(isArray()) {
			return m_field.arrayValue;
		}
		return NULL;
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
	bool result(CCValue result){return callback("", result);};
	bool error(std::string err){return callback(err, nullValue());};

private:
    CCValueField m_field;
	bool m_retain;
	std::string	 m_fieldString;
    CCValueType  m_type;

    void copy(const CCValue& rhs);

	friend class CCValueBuilder;
};

typedef struct _CCVBItem {
	CCValueField field;
	std::string	 strval;
	std::map<std::string, void*> mapval;
	std::vector<void*> arrayval;
    CCValueType  type;
	void* parent;
} CCVBItem;

class CC_DLL CCValueBuilder
{
public:
	CCValueBuilder();
	~CCValueBuilder();

	CCValueBuilder* beNull();
    CCValueBuilder* beInt(const int intValue);
    CCValueBuilder* beNumber(const double numberValue);
    CCValueBuilder* beBoolean(const bool booleanValue);
    CCValueBuilder* beString(const char* stringValue);
    CCValueBuilder* beString(const std::string& stringValue);    
    CCValueBuilder* beObject(CCObject* obj);
	CCValueBuilder* beFCall(CC_FUNCTION_CALL call);
	CCValueBuilder* beOCall(CCObject* obj, CC_OBJECT_CALL call);

	CCValueBuilder* mapBegin();
	CCValueBuilder* addMap(const char* key);
	CCValueBuilder* mapEnd();
	CCValueBuilder* arrayBegin();
	CCValueBuilder* addArray();
	CCValueBuilder* arrayEnd();

	void build(CCValue* root);
	void build(CCValueArray* root);

protected:
	CCVBItem* current();
	void push();
	void pop();
	void deleteItem(CCVBItem* item);

	void build(CCVBItem* item, CCValue* pval);
	void build(CCVBItem* item, CCValueArray* pval);

	CCValue* arrayAddV(CCValueArray* arr);
	CCValue* mapAddV(CCValueMap* map, const char* key);

protected:
	CCVBItem* m_parent;
	CCVBItem* m_current;
};

class CC_DLL CCValueReader
{
public:
	CCValueReader(CCValue* value);
	~CCValueReader();

	CCValue* value();

	bool isMap();
	CCValue* get(const char* name);
	CCValue* getNull(const char* name);
	void remove(const char* name);
	bool beMap(const char* name);
	bool beArray(const char* name);
	
	bool isArray();
	int arraySize();
	CCValue* get(int idx);
	CCValue* getNull(int idx);
	bool beMap(int idx);
	bool beArray(int idx);
	
	void pop();

protected:
	std::vector<CCValue*> m_stack;
	CCValue* m_value;
};

NS_CC_END

#endif // __CCVALUE_H__
